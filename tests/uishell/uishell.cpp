/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

// Starts each vendored UI toolkit up and down once with no window, renderer, or game
// data, linked the way the engine links them. A runtime library or structure layout
// mismatch shows here as a link error or a failed check.

#include <cstdio>

#include <RmlUi/Core.h>
#include <ft2build.h>
#include FT_FREETYPE_H
#include <imgui.h>

namespace {

int Failures = 0;


void Check(bool condition, char const * what)
{
	std::printf("%-76s %s\n", what, condition ? "ok" : "FAILED");

	if (!condition) {
		Failures++;
	}
}


void Test_RmlUi(void)
{
	Check(Rml::Initialise(), "RmlUi initialises without a render interface");

	Rml::String version = Rml::GetVersion();
	std::printf("  RmlUi %s\n", version.c_str());
	Check(!version.empty(), "RmlUi reports a version");

	Rml::Shutdown();
}


void Test_FreeType(void)
{
	FT_Library library = nullptr;
	Check(FT_Init_FreeType(&library) == 0 && library != nullptr, "FreeType initialises a library");

	if (library != nullptr) {
		FT_Int major = 0;
		FT_Int minor = 0;
		FT_Int patch = 0;
		FT_Library_Version(library, &major, &minor, &patch);
		std::printf("  FreeType %d.%d.%d\n", major, minor, patch);
		Check(major == 2, "FreeType reports the 2.x API");

		FT_Done_FreeType(library);
	}
}


void Test_ImGui(void)
{
	Check(IMGUI_CHECKVERSION(), "ImGui header and library agree on structure layouts");

	ImGuiContext * context = ImGui::CreateContext();
	Check(context != nullptr, "ImGui creates a context");
	std::printf("  Dear ImGui %s\n", ImGui::GetVersion());

	if (context != nullptr) {
		ImGui::DestroyContext(context);
	}
}

}


int main(void)
{
	Test_RmlUi();
	Test_FreeType();
	Test_ImGui();

	std::printf("\n%s\n", Failures == 0 ? "PASSED" : "FAILED");
	return(Failures == 0 ? 0 : 1);
}
