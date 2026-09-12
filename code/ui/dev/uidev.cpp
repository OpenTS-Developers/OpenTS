/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#include "ui/dev/uidev.h"

#include "_bench.h"
#include "bench.h"
#include "bench.hh"
#include "dbgprint.h"
#include "globals.h"
#include "logic.h"
#include "mono.h"
#include "mpu.h"
#include "ui/rml/rmlrender.h"
#include "video.h"

#include <imgui.h>

#include <cstdio>


static ImGuiContext * _Context = NULL;
static bool _Shown = false;
static bool _ShowDemo = false;
static long long _LastFrameTicks = 0;

// The processor speed in megahertz, measured once when the window first opens; zero when
// the machine could not say.
static int _CPUSpeed = 0;

// The benchmarks as they stood when they were last reset, so the table reports the second
// just gone the way the monochrome page does rather than a second half counted.
struct UIBenchmarkSample
{
	unsigned int Value;
	unsigned int Count;
};

static UIBenchmarkSample _Samples[BENCH_COUNT];
static bool _SamplesValid = false;
static bool _ResetEachSecond = true;
static unsigned int _LastSampleTime = 0;

static ImGuiKey _KeyMap[256];


struct UIDevKeyMapping
{
	int VirtualKey;
	ImGuiKey Key;
};

static const UIDevKeyMapping _KeyMappings[] = {
	{ VK_TAB, ImGuiKey_Tab },
	{ VK_LEFT, ImGuiKey_LeftArrow },
	{ VK_RIGHT, ImGuiKey_RightArrow },
	{ VK_UP, ImGuiKey_UpArrow },
	{ VK_DOWN, ImGuiKey_DownArrow },
	{ VK_PRIOR, ImGuiKey_PageUp },
	{ VK_NEXT, ImGuiKey_PageDown },
	{ VK_HOME, ImGuiKey_Home },
	{ VK_END, ImGuiKey_End },
	{ VK_INSERT, ImGuiKey_Insert },
	{ VK_DELETE, ImGuiKey_Delete },
	{ VK_BACK, ImGuiKey_Backspace },
	{ VK_SPACE, ImGuiKey_Space },
	{ VK_RETURN, ImGuiKey_Enter },
	{ VK_ESCAPE, ImGuiKey_Escape },
	{ VK_OEM_7, ImGuiKey_Apostrophe },
	{ VK_OEM_COMMA, ImGuiKey_Comma },
	{ VK_OEM_MINUS, ImGuiKey_Minus },
	{ VK_OEM_PERIOD, ImGuiKey_Period },
	{ VK_OEM_2, ImGuiKey_Slash },
	{ VK_OEM_1, ImGuiKey_Semicolon },
	{ VK_OEM_PLUS, ImGuiKey_Equal },
	{ VK_OEM_4, ImGuiKey_LeftBracket },
	{ VK_OEM_5, ImGuiKey_Backslash },
	{ VK_OEM_6, ImGuiKey_RightBracket },
	{ VK_OEM_3, ImGuiKey_GraveAccent },
	{ VK_CAPITAL, ImGuiKey_CapsLock },
	{ VK_SCROLL, ImGuiKey_ScrollLock },
	{ VK_NUMLOCK, ImGuiKey_NumLock },
	{ VK_SNAPSHOT, ImGuiKey_PrintScreen },
	{ VK_PAUSE, ImGuiKey_Pause },
	{ VK_NUMPAD0, ImGuiKey_Keypad0 },
	{ VK_NUMPAD1, ImGuiKey_Keypad1 },
	{ VK_NUMPAD2, ImGuiKey_Keypad2 },
	{ VK_NUMPAD3, ImGuiKey_Keypad3 },
	{ VK_NUMPAD4, ImGuiKey_Keypad4 },
	{ VK_NUMPAD5, ImGuiKey_Keypad5 },
	{ VK_NUMPAD6, ImGuiKey_Keypad6 },
	{ VK_NUMPAD7, ImGuiKey_Keypad7 },
	{ VK_NUMPAD8, ImGuiKey_Keypad8 },
	{ VK_NUMPAD9, ImGuiKey_Keypad9 },
	{ VK_DECIMAL, ImGuiKey_KeypadDecimal },
	{ VK_DIVIDE, ImGuiKey_KeypadDivide },
	{ VK_MULTIPLY, ImGuiKey_KeypadMultiply },
	{ VK_SUBTRACT, ImGuiKey_KeypadSubtract },
	{ VK_ADD, ImGuiKey_KeypadAdd },
	{ VK_SHIFT, ImGuiKey_LeftShift },
	{ VK_LSHIFT, ImGuiKey_LeftShift },
	{ VK_RSHIFT, ImGuiKey_RightShift },
	{ VK_CONTROL, ImGuiKey_LeftCtrl },
	{ VK_LCONTROL, ImGuiKey_LeftCtrl },
	{ VK_RCONTROL, ImGuiKey_RightCtrl },
	{ VK_MENU, ImGuiKey_LeftAlt },
	{ VK_LMENU, ImGuiKey_LeftAlt },
	{ VK_RMENU, ImGuiKey_RightAlt },
	{ VK_LWIN, ImGuiKey_LeftSuper },
	{ VK_RWIN, ImGuiKey_RightSuper },
	{ VK_APPS, ImGuiKey_Menu },
	{ '0', ImGuiKey_0 },
	{ '1', ImGuiKey_1 },
	{ '2', ImGuiKey_2 },
	{ '3', ImGuiKey_3 },
	{ '4', ImGuiKey_4 },
	{ '5', ImGuiKey_5 },
	{ '6', ImGuiKey_6 },
	{ '7', ImGuiKey_7 },
	{ '8', ImGuiKey_8 },
	{ '9', ImGuiKey_9 },
	{ 'A', ImGuiKey_A },
	{ 'B', ImGuiKey_B },
	{ 'C', ImGuiKey_C },
	{ 'D', ImGuiKey_D },
	{ 'E', ImGuiKey_E },
	{ 'F', ImGuiKey_F },
	{ 'G', ImGuiKey_G },
	{ 'H', ImGuiKey_H },
	{ 'I', ImGuiKey_I },
	{ 'J', ImGuiKey_J },
	{ 'K', ImGuiKey_K },
	{ 'L', ImGuiKey_L },
	{ 'M', ImGuiKey_M },
	{ 'N', ImGuiKey_N },
	{ 'O', ImGuiKey_O },
	{ 'P', ImGuiKey_P },
	{ 'Q', ImGuiKey_Q },
	{ 'R', ImGuiKey_R },
	{ 'S', ImGuiKey_S },
	{ 'T', ImGuiKey_T },
	{ 'U', ImGuiKey_U },
	{ 'V', ImGuiKey_V },
	{ 'W', ImGuiKey_W },
	{ 'X', ImGuiKey_X },
	{ 'Y', ImGuiKey_Y },
	{ 'Z', ImGuiKey_Z },
	{ VK_F1, ImGuiKey_F1 },
	{ VK_F2, ImGuiKey_F2 },
	{ VK_F3, ImGuiKey_F3 },
	{ VK_F4, ImGuiKey_F4 },
	{ VK_F5, ImGuiKey_F5 },
	{ VK_F6, ImGuiKey_F6 },
	{ VK_F7, ImGuiKey_F7 },
	{ VK_F8, ImGuiKey_F8 },
	{ VK_F9, ImGuiKey_F9 },
	{ VK_F10, ImGuiKey_F10 },
	{ VK_F11, ImGuiKey_F11 },
	{ VK_F12, ImGuiKey_F12 },
};


// The rows of the benchmark table, in the groups the monochrome Events page uses. Five of
// the counters are declared but never started anywhere in the engine.
struct UIBenchmarkRow
{
	BenchType Type;
	char const * Group;
	char const * Name;
	bool Instrumented;
};

static const UIBenchmarkRow _Rows[] = {
	{ BENCH_FINDPATH, "Logic", "Find path", true },
	{ BENCH_GREATEST_THREAT, "Logic", "Greatest threat", true },
	{ BENCH_AI, "Logic", "Object AI", true },
	{ BENCH_PCP, "Logic", "Per cell process", true },
	{ BENCH_EVAL_OBJECT, "Logic", "Evaluate object", true },
	{ BENCH_EVAL_CELL, "Logic", "Evaluate cell", true },
	{ BENCH_EVAL_WALL, "Logic", "Evaluate wall", true },
	{ BENCH_MISSION, "Logic", "Mission list", true },
	{ BENCH_CELL, "Map objects", "Cell drawing", true },
	{ BENCH_OBJECTS, "Map objects", "Object drawing", false },
	{ BENCH_ANIMS, "Map objects", "Animations", true },
	{ BENCH_PALETTE, "Palette", "Color cycling", false },
	{ BENCH_GSCREEN_RENDER, "Presentation", "Screen render", true },
	{ BENCH_SIDEBAR, "Presentation", "Sidebar cameos", true },
	{ BENCH_RADAR, "Presentation", "Radar", false },
	{ BENCH_TACTICAL, "Presentation", "Tactical map", false },
	{ BENCH_POWER, "Presentation", "Power bar", true },
	{ BENCH_SHROUD, "Presentation", "Shroud", false },
	{ BENCH_TABS, "Presentation", "Tabs", true },
	{ BENCH_BLIT_DISPLAY, "Presentation", "Blit to display", true },
};


static void Build_Key_Map(void)
{
	for (int code = 0; code < 256; code++) {
		_KeyMap[code] = ImGuiKey_None;
	}

	for (UIDevKeyMapping const & mapping : _KeyMappings) {
		_KeyMap[mapping.VirtualKey] = mapping.Key;
	}
}


// The aggregate modifier a sided key contributes to; ImGui wants both submitted.
static ImGuiKey Modifier_Of(ImGuiKey key)
{
	switch (key) {
		case ImGuiKey_LeftShift:
		case ImGuiKey_RightShift:
			return(ImGuiMod_Shift);

		case ImGuiKey_LeftCtrl:
		case ImGuiKey_RightCtrl:
			return(ImGuiMod_Ctrl);

		case ImGuiKey_LeftAlt:
		case ImGuiKey_RightAlt:
			return(ImGuiMod_Alt);

		case ImGuiKey_LeftSuper:
		case ImGuiKey_RightSuper:
			return(ImGuiMod_Super);

		default:
			return(ImGuiKey_None);
	}
}


static bool Snapshots_In_Use(void)
{
	return(_ResetEachSecond && !MonoClass::Is_Enabled());
}


// Copies every counter aside and starts the next second, leaving the load timings alone.
static void Sample_Benchmarks(void)
{
	for (int index = BENCH_FIRST; index < BENCH_COUNT; index++) {
		_Samples[index].Value = Benches[index].Value();
		_Samples[index].Count = Benches[index].Count();
		if (index != BENCH_RULES && index != BENCH_SCENARIO) {
			Benches[index].Reset();
		}
	}

	_SamplesValid = true;
	_LastSampleTime = timeGetTime();
}


static UIBenchmarkSample Read_Benchmark(BenchType type)
{
	if (Snapshots_In_Use() && _SamplesValid) {
		return(_Samples[type]);
	}

	UIBenchmarkSample sample;
	sample.Value = Benches[type].Value();
	sample.Count = Benches[type].Count();
	return(sample);
}


// An average in microseconds when the processor speed is known; the raw RDTSC/16 ticks
// otherwise.
static void Format_Average(char * buffer, size_t size, unsigned int ticks)
{
	if (_CPUSpeed > 0) {
		std::snprintf(buffer, size, "%.1f", (double)ticks * 16.0 / (double)_CPUSpeed);
	} else {
		std::snprintf(buffer, size, "%u", ticks);
	}
}


static void Draw_Benchmark_Table(void)
{
	UIBenchmarkSample frame = Read_Benchmark(BENCH_GAME_FRAME);
	double frametotal = (double)frame.Value * (double)frame.Count;

	if (!ImGui::BeginTable("benchmarks", 4, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingStretchProp)) {
		return;
	}

	ImGui::TableSetupColumn("Process");
	ImGui::TableSetupColumn("Frame %");
	ImGui::TableSetupColumn(_CPUSpeed > 0 ? "Average (us)" : "Average (ticks)");
	ImGui::TableSetupColumn("Samples");
	ImGui::TableHeadersRow();

	char const * group = NULL;
	char average[32];

	for (UIBenchmarkRow const & row : _Rows) {
		if (group == NULL || strcmp(group, row.Group) != 0) {
			group = row.Group;
			ImGui::TableNextRow();
			ImGui::TableNextColumn();
			ImGui::TextDisabled("%s", group);
		}

		ImGui::TableNextRow();
		ImGui::TableNextColumn();
		ImGui::TextUnformatted(row.Name);

		if (!row.Instrumented) {
			ImGui::TableNextColumn();
			ImGui::TextDisabled("not instrumented");
			continue;
		}

		UIBenchmarkSample sample = Read_Benchmark(row.Type);
		double own = (double)sample.Value * (double)sample.Count;
		double percent = frametotal > 0.0 ? own * 100.0 / frametotal : 0.0;
		if (percent > 100.0) {
			percent = 100.0;
		}

		ImGui::TableNextColumn();
		ImGui::Text("%.1f", percent);
		ImGui::TableNextColumn();
		Format_Average(average, sizeof(average), sample.Value);
		ImGui::TextUnformatted(average);
		ImGui::TableNextColumn();
		ImGui::Text("%u", sample.Count);
	}

	ImGui::EndTable();
}


static void Draw_Benchmark_Window(void)
{
	ImGui::SetNextWindowPos(ImVec2(16.0f, 16.0f), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(560.0f, 620.0f), ImGuiCond_FirstUseEver);

	if (!ImGui::Begin("Frame benchmarks", &_Shown)) {
		ImGui::End();
		return;
	}

	ImGui::Text("Logic frames per second %u, frame %d", LastFramesPerSecond, Frame);
	ImGui::Text("Presents per second %u, present interval %u ms", Video_Presents_Per_Second(), Video_Present_Interval());
	ImGui::Text("Overlay ticks per second %.0f", ImGui::GetIO().Framerate);
	ImGui::Checkbox("Show the Dear ImGui demo window", &_ShowDemo);
	ImGui::Separator();

	if (Benches == NULL) {
		ImGui::TextUnformatted("The benchmarks are compiled into Debug builds only.");
		ImGui::End();
		return;
	}

	if (ImGui::Button("Reset")) {
		Sample_Benchmarks();
	}
	ImGui::SameLine();
	ImGui::Checkbox("Reset every second", &_ResetEachSecond);
	if (_ResetEachSecond && MonoClass::Is_Enabled()) {
		ImGui::SameLine();
		ImGui::TextDisabled("(the monochrome display owns the reset while it is on)");
	}

	if (Snapshots_In_Use() && (!_SamplesValid || timeGetTime() - _LastSampleTime >= 1000)) {
		Sample_Benchmarks();
	}

	Draw_Benchmark_Table();

	char rules[32];
	char scenario[32];
	Format_Average(rules, sizeof(rules), Benches[BENCH_RULES].Value());
	Format_Average(scenario, sizeof(scenario), Benches[BENCH_SCENARIO].Value());
	ImGui::Text("Load times: rules %s, scenario %s", rules, scenario);

	ImGui::End();
}


bool UIDev_Active(void)
{
	return(_Context != NULL && _Shown);
}


void UIDev_Toggle(UIRmlRenderClass const & render)
{
	if (_Context == NULL) {
		IMGUI_CHECKVERSION();
		_Context = ImGui::CreateContext();

		ImGuiIO & io = ImGui::GetIO();
		io.IniFilename = NULL;
		io.LogFilename = NULL;
		io.BackendFlags |= ImGuiBackendFlags_RendererHasTextures | ImGuiBackendFlags_RendererHasVtxOffset;
		io.ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;
		io.BackendPlatformName = "OpenTS shell";
		io.BackendRendererName = "OpenTS bgfx";

		ImGuiPlatformIO & platform = ImGui::GetPlatformIO();
		platform.Renderer_TextureMaxWidth = render.Texture_Limit();
		platform.Renderer_TextureMaxHeight = render.Texture_Limit();

		Build_Key_Map();
		_CPUSpeed = Get_RDTSC_CPU_Speed();
		_LastFrameTicks = 0;
		_SamplesValid = false;

		DebugString("UI: Dear ImGui %s context created, processor %d MHz\n", ImGui::GetVersion(), _CPUSpeed);
	}

	_Shown = !_Shown;
	render.Log_Resource_Counts(_Shown ? "developer overlay shown" : "developer overlay hidden");
}


// One tick is one ImGui frame. The scale can change between frames, so the display size
// and the font scale follow the frame's destination every time.
void UIDev_Tick(void)
{
	if (!UIDev_Active()) {
		return;
	}

	VideoScaleInfo const & scale = Video_Get_Scale_Info();
	if (scale.DestWidth <= 0 || scale.DestHeight <= 0) {
		return;
	}

	float ratio = scale.ScaleX < scale.ScaleY ? scale.ScaleX : scale.ScaleY;
	if (ratio <= 0.0f) {
		ratio = 1.0f;
	}

	LARGE_INTEGER now;
	LARGE_INTEGER frequency;
	QueryPerformanceCounter(&now);
	QueryPerformanceFrequency(&frequency);
	float delta = (_LastFrameTicks == 0 || frequency.QuadPart == 0) ? (1.0f / 60.0f) : (float)(now.QuadPart - _LastFrameTicks) / (float)frequency.QuadPart;
	if (delta < 0.0001f) {
		delta = 0.0001f;
	}
	_LastFrameTicks = now.QuadPart;

	ImGuiIO & io = ImGui::GetIO();
	io.DisplaySize = ImVec2((float)scale.DestWidth, (float)scale.DestHeight);
	io.DeltaTime = delta;
	ImGui::GetStyle().FontScaleDpi = ratio;

	ImGui::NewFrame();
	Draw_Benchmark_Window();
	if (_ShowDemo) {
		ImGui::ShowDemoWindow(&_ShowDemo);
	}
	ImGui::Render();
}


void UIDev_Render(UIRmlRenderClass & render)
{
	if (!UIDev_Active()) {
		return;
	}

	render.Render_ImGui(ImGui::GetDrawData());
}


void UIDev_Shutdown(UIRmlRenderClass & render)
{
	if (_Context == NULL) {
		return;
	}

	render.Destroy_ImGui_Textures();
	ImGui::DestroyContext(_Context);
	_Context = NULL;
	_Shown = false;
	_ShowDemo = false;
}


void UIDev_Mouse_Position(int x, int y)
{
	if (!UIDev_Active()) {
		return;
	}

	ImGui::GetIO().AddMousePosEvent((float)x, (float)y);
}


bool UIDev_Mouse_Button(int button, bool down)
{
	if (!UIDev_Active()) {
		return(false);
	}

	ImGuiIO & io = ImGui::GetIO();
	io.AddMouseButtonEvent(button, down);
	return(io.WantCaptureMouse);
}


bool UIDev_Mouse_Wheel(float delta)
{
	if (!UIDev_Active()) {
		return(false);
	}

	ImGuiIO & io = ImGui::GetIO();
	io.AddMouseWheelEvent(0.0f, delta);
	return(io.WantCaptureMouse);
}


bool UIDev_Key(WPARAM virtualkey, bool down)
{
	if (!UIDev_Active()) {
		return(false);
	}

	ImGuiIO & io = ImGui::GetIO();
	ImGuiKey key = _KeyMap[virtualkey & 0xFF];
	if (key != ImGuiKey_None) {
		ImGuiKey modifier = Modifier_Of(key);
		if (modifier != ImGuiKey_None) {
			io.AddKeyEvent(modifier, down);
		}
		io.AddKeyEvent(key, down);
	}

	return(io.WantCaptureKeyboard);
}


bool UIDev_Character(wchar_t unit)
{
	if (!UIDev_Active()) {
		return(false);
	}

	ImGuiIO & io = ImGui::GetIO();
	io.AddInputCharacterUTF16((ImWchar16)unit);
	return(io.WantTextInput);
}


void UIDev_Focus(bool focused)
{
	if (_Context == NULL) {
		return;
	}

	ImGui::GetIO().AddFocusEvent(focused);
}


bool UIDev_Wants_Mouse(void)
{
	return(UIDev_Active() && ImGui::GetIO().WantCaptureMouse);
}
