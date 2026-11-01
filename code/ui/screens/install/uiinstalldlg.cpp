/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#include "always.h"

#include "_ui.h"
#include "side.hh"
#include "ui/screens/install/uiinstall.h"
#include "ui/uienginehost.h"
#include "ui/uishell.h"
#include "ui/uiview.h"


UIInstallResult UI_Install_Dialog(void)
{
	UIInstallPresenterClass presenter;
	std::unique_ptr<UIViewClass> view = UI_Install_View(presenter);

	UIResult result = UIShell.Run_Modal(*view, [&presenter](void) {
		bool ended = UI_Service_Game();
		if (presenter.Busy && presenter.Transfer.Active()) {
			presenter.Transfer.Tick();
			presenter.Status = presenter.Transfer.Status();
			presenter.Percent = presenter.Transfer.Percent();
			if (!presenter.Transfer.Active()) {
				if (presenter.Transfer.Succeeded()) {
					presenter.Busy = false;
					presenter.Status.clear();
					presenter.Installed = true;
				} else {
					presenter.Busy = false;
					presenter.Error = presenter.Transfer.Error();
					presenter.HasError = !presenter.Error.empty();
					presenter.Status.clear();
				}
			}
		}
		return(ended);
	});

	// The install screen renders before the game data is mounted, so its controls read the art
	// as missing and cache that. Drop the cache so the next screen is rebuilt from the mounted data.
	UI_On_Archives_Change(SIDE_NONE);

	if (result == UI_RESULT_ACCEPTED && presenter.Restart) {
		return(UI_INSTALL_RESTART);
	}
	if (result == UI_RESULT_CANCELLED) {
		return(UI_INSTALL_QUIT);
	}
	return(UI_INSTALL_UNAVAILABLE);
}
