/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#include "_ui.h"
#include "ui/uishell.h"
#include "always.h"

#include "grphmenu.h"

#include "_keyboar.h"
#include "_surface.h"
#include "ccfile.h"
#include "globals.h"
#include "grphmitm.h"
#include "ini.h"
#include "keyboard.h"
#include "msanim.h"
#include "ownrdraw.h"
#include "theme.h"

GraphicMenu * _Graphic_Menu(INIClass const & ini, const char * name);
GraphicMenuItem * GM_Create_Item_From_INI(const char * name, INIClass const & ini, MSEngine & engine, Point2D & image_size);


/// <summary>
/// Creates a graphic menu described by an INI file.
/// Use this routine to build one of the shell menu pages. The file is fetched through
/// the mix file system and the named section within it describes the page. The menu is
/// only built here -- the caller must run it and dispose of it.
/// </summary>
/// <param name="ini">The name of the INI file that holds the menu descriptions.</param>
/// <param name="name">The name of the section that describes this menu.</param>
/// <returns>Returns with a pointer to the menu built, or NULL if the file is missing or
/// unreadable.</returns>
GraphicMenu * Do_Graphic_Menu(const char * ini, const char * name)
{
	CCFileClass file(ini);
	if (!file.Is_Available()) {
		return(NULL);
	}
	INIClass _ini;
	GraphicMenu * menu = _ini.Load(file) > 0 ? _Graphic_Menu(_ini, name) : NULL;
	return(menu);
}


/// <summary>
/// Creates a graphic menu from an INI section.
/// This routine builds the menu, gives it whatever backdrop animation and music theme
/// the section calls for, then asks the item factory to create each listed item in
/// turn. A missing backdrop animation is not fatal -- the menu simply has none.
/// </summary>
/// <param name="ini">The database to read the menu description from.</param>
/// <param name="name">The name of the section that describes this menu.</param>
/// <returns>Returns with a pointer to the menu built, or NULL if it could not be
/// allocated.</returns>
GraphicMenu * _Graphic_Menu(INIClass const & ini, const char * name)
{
	GraphicMenu * menu = new GraphicMenu;
	char buffer[256];

	if (menu == NULL) {
		return(NULL);
	}

	bool has_background = ini.Get_String(name, "Background", "", buffer, sizeof(buffer)) > 0;
	menu->BackgroundName.Replace_With_Extension(buffer, ".PCX", sizeof(".PCX") - 1);

	Point2D pt(0,0);

	if (has_background) {
		strncat(buffer, ".VQA", sizeof(buffer) - strlen(buffer) - 1);
		MSAnim * anim = NULL;
		if (CCFileClass(buffer).Is_Available()) {
			anim = new MSVQAnim(buffer, AlternateSurface, menu->Engine.Get_Anims(), true);
		}
		if (anim == NULL) {
			anim = new MSPCXAnim(buffer, menu->Engine.Get_Anims(), true);
		}

		menu->Set_Animation(anim);
		pt = anim->Get_Rect().TopLeft;
	}

	if (ini.Get_String(name, "Theme", "", buffer, sizeof(buffer)) > 0) {
		menu->Set_Theme_Name(buffer);
	}

	char entry[8];
	int item_max = ini.Get_Int(name, "ItemMax", 100);
	for (int i = 0; i <= item_max; i++) {
		sprintf(entry, "%d", i);
		if (ini.Get_String(name, entry, "", buffer, sizeof(buffer)) > 0) {
			GraphicMenuItem * item = GM_Create_Item_From_INI(buffer, ini, menu->Engine, pt);
			if (item != NULL) {
				menu->Add_Item(item);
			}
		}
	}

	return(menu);
}


/// <summary>
/// Creates an empty graphic menu.
/// The menu comes up with the title screen behind it and the intro music playing. The
/// INI reader supplies the real backdrop, theme, and items afterward.
/// </summary>
GraphicMenu::GraphicMenu(void) :
	Engine(),
	Items(),
	CurrentAnim(NULL)
{
	BackgroundName.Set("Title.PCX");
	ThemeName.Set("Intro");
}


/// <summary>
/// Destroys the menu along with everything on it.
/// The menu owns the items that were added to it, so they are deleted here.
/// </summary>
GraphicMenu::~GraphicMenu(void)
{
	BackgroundName.Release();
	ThemeName.Release();

	for (GraphicMenuItem * item : Items) {
		delete item;
	}
}


/// <summary>
/// Enables or disables the menu items carrying an identifier.
/// The shell menu handler uses this routine to shut off the choices that do not apply
/// to the page it is about to display.
/// </summary>
/// <param name="id">The identifier of the items to change.</param>
/// <param name="enabled">Should the items be enabled?</param>
void GraphicMenu::Set_Item_Enabled(int id, bool enabled)
{
	for (GraphicMenuItem * item : Items) {
		if (item->Get_ID() == id) {
			item->Set_Enabled(enabled);
		}
	}
}


/// <summary>
/// Runs the menu until the player picks something.
/// This routine starts the menu's theme and then takes over the mouse and keyboard,
/// highlighting whichever item the player is pointing at, until an item is chosen.
/// The chosen item performs its action before control is handed back.
/// </summary>
/// <returns>Returns with the identifier of the menu item the player chose.</returns>
int GraphicMenu::Presentation(void)
{
	Theme.Play_Song(Theme.From_Name(ThemeName.Peek()));

	OwnerDraw::Capture_Mouse();

	HiddenSurface->Fill(0);
	AlternateSurface->Fill(0);

	bool done = false;
	GraphicMenuItem * item = NULL;

	Keyboard->Clear();

	if (CurrentAnim != NULL) {
		Engine.Wait_For_Anim(CurrentAnim);
	}

	Engine.Restore_Anims(AlternateSurface->Get_Rect());
	Engine.Restore_And_Advance();

	while (!done) {
		Hide_Mouse();
		Engine.Wait_For_Focus();
		Show_Mouse();

		Point2D mouse(Get_Mouse_X(), Get_Mouse_Y());

		if (Keyboard->Check() != KN_NONE) {
			KeyNumType key = Keyboard->Get();
			GraphicMenuItem * temp = (key == KN_LMOUSE || key == KN_RETURN) ? Get_Item_Under_Mouse(mouse) : Get_Item_For_Key(key);

			if (temp != NULL) {
				if (item != temp) {
					if (item != NULL) {
						item->Set_Selected(false);
					}
					item = temp;
					if (temp != NULL) {
						temp->Set_Selected(true);
					}
				}
				done = true;
			}
		} else {
			GraphicMenuItem * temp = Get_Item_Under_Mouse(mouse);
			if (temp != item) {
				if (item != NULL) {
					item->Set_Selected(false);
				}
				item = temp;
				if (temp != NULL) {
					temp->Set_Selected(true);
				}
			}
		}

		// The menu runs its own loop, so nothing else advances a document or an overlay
		// while it is up.
		UIShell.Tick();

		Engine.Wait_Delay(1);
	}

	if (item != NULL) {
		item->Action(&Engine);
	}

	OwnerDraw::Release_Mouse();

	Theme.Fade_Out();

	if (item != NULL) {
		return(item->Get_ID());
	}

	return(-1);
}


/// <summary>
/// Sets the animation that plays behind the menu.
/// The animation is handed to the menu's animation engine, taking the place of
/// whatever backdrop was there before.
/// </summary>
void GraphicMenu::Set_Animation(MSAnim *anim)
{
	MSAnim * old = CurrentAnim;
	if (old != NULL) {
		Engine.Replace_Anim(old, anim);
	} else {
		Engine.Add_Animation(anim);
	}
	CurrentAnim = anim;
}


/// <summary>
/// Sets the music theme that plays while this menu is up.
/// </summary>
void GraphicMenu::Set_Theme_Name(const char * name)
{
	ThemeName.Replace(name);
}


/// <summary>
/// Adds an item to the menu.
/// </summary>
/// <remarks>The menu takes ownership of the item and will delete it when the menu is
/// destroyed.</remarks>
void GraphicMenu::Add_Item(GraphicMenuItem * item)
{
	Items.Add(item);
}


/// <summary>
/// Fetches the menu item lying under the mouse.
/// </summary>
/// <returns>Returns with a pointer to the item beneath the mouse, or NULL if the mouse
/// is over none of them.</returns>
GraphicMenuItem * GraphicMenu::Get_Item_Under_Mouse(Point2D const & mouse)
{
	for (GraphicMenuItem * item : Items) {
		if (item->Is_Mouse_Over(mouse)) {
			return(item);
		}
	}
	return(NULL);
}


/// <summary>
/// Fetches the menu item bound to the specified key.
/// </summary>
/// <returns>Returns with a pointer to the item that answers to the key, or NULL if no
/// item claims it.</returns>
GraphicMenuItem * GraphicMenu::Get_Item_For_Key(KeyNumType key)
{
	for (GraphicMenuItem * item : Items) {
		if (item->Is_Input_Key(key)) {
			return(item);
		}
	}
	return(NULL);
}
