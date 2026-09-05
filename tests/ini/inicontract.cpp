/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

// Pins the INI reader's contract with no engine and no game data: how a repeated section
// or key resolves, what Save writes back (which is also what the message digest hashes),
// what a malformed number reads as, which repeats are reported, and how fast a big file
// loads.

#include <algorithm>
#include <chrono>
#include <cstdio>
#include <cstring>
#include <string>

#include "crc.h"
#include "dbgprint.h"
#include "ini.h"
#include "point.h"
#include "rect.h"
#include "wwfile.h"
#include "xpipe.h"
#include "xstraw.h"

int Test_UUBlock(void);

namespace {

int Failures = 0;


void Check(bool condition, char const * what)
{
	std::printf("%-76s %s\n", what, condition ? "ok" : "FAILED");

	if (!condition) {
		Failures++;
	}
}


void Read(INIClass & ini, char const * text, bool keepcomments = false)
{
	BufferStraw straw(text, (int)std::strlen(text));
	ini.Load(straw, keepcomments);
}


bool Value_Is(INIClass const & ini, char const * section, char const * entry, char const * expected)
{
	char buffer[INIClass::MAX_LINE_LENGTH];
	ini.Get_String(section, entry, "<missing>", buffer, sizeof(buffer));
	return(std::strcmp(buffer, expected) == 0);
}


bool Entry_Is(INIClass const & ini, char const * section, int index, char const * expected)
{
	char const * name = ini.Get_Entry(section, index);
	return(name != NULL && std::strcmp(name, expected) == 0);
}


std::string Save_Bytes(INIClass const & ini)
{
	std::string bytes(1 << 20, '\0');
	BufferPipe pipe(bytes.data(), (int)bytes.size());
	int total = ini.Save(pipe);
	bytes.resize(total > 0 ? total : 0);
	return(bytes);
}


// Counts the lines of the debug log that carry the text, so a test can ask how many
// diagnostics a load produced by taking the count before and after it.
int Log_Lines(char const * needle)
{
	FILE * handle = std::fopen(Debug_Log_File_Name(), "rb");
	if (handle == NULL) {
		return(0);
	}

	int count = 0;
	char line[4096];
	while (std::fgets(line, sizeof(line), handle) != NULL) {
		if (std::strstr(line, needle) != NULL) {
			count++;
		}
	}
	std::fclose(handle);
	return(count);
}


// A file held in memory, so that a load can be given a file name to report.
class MemoryFileClass : public FileClass
{
	public:
		MemoryFileClass(char const * name, char const * text) : Name(name), Text(text), Offset(0), Opened(false) {}

		char const * File_Name(void) const override {return(Name.c_str());}
		char const * Set_Name(char const * filename) override {Name = filename; return(Name.c_str());}
		int Create(void) override {return(false);}
		int Delete(void) override {return(false);}
		bool Is_Available(int = false) override {return(true);}
		bool Is_Open(void) const override {return(Opened);}
		int Open(char const *, int rights) override {return(Open(rights));}
		int Open(int) override {Opened = true; Offset = 0; return(true);}
		int Read(void * buffer, int size) override
		{
			int count = std::min(size, (int)(Text.size() - Offset));
			std::memcpy(buffer, Text.data() + Offset, count);
			Offset += count;
			return(count);
		}
		int Seek(int pos, int dir) override
		{
			if (dir == SEEK_SET) Offset = pos;
			else if (dir == SEEK_CUR) Offset += pos;
			else Offset = (int)Text.size() + pos;
			return(Offset);
		}
		int Size(void) override {return((int)Text.size());}
		int Write(void const *, int) override {return(0);}
		void Close(void) override {Opened = false;}
		void Error(int, int, char const *) override {}

	private:
		std::string Name;
		std::string Text;
		int Offset;
		bool Opened;
};


// The laser fence post as one player's edited rules file had it. The repeated Turret line
// and the stray comment between the two are copied from the file that aborted a debug build
// while Init_Rules read it; the second value differs here so the test can tell which won.
char const _Repeated_Entry[] =
	"[NALASR]\n"
	"Name=Laser\n"
	"Primary=LaserFire2\n"
	"Turret=yes\n"
	";\n"
	"Turret=no\n"
	"TurretAnim=NALASR\n";


// The same section header written twice, each block carrying its own keys.
char const _Repeated_Section[] =
	"[General]\n"
	"Crate=10\n"
	"TiberiumGrowth=0.02\n"
	"\n"
	"[General]\n"
	"TiberiumGrowth=0.09\n"
	"BridgeVoxelMax=10\n";


// A file with nothing repeated in it, and the bytes Save writes for it.
char const _Plain[] =
	"[General]\n"
	"Crate=10\n"
	"TiberiumGrowth=0.02\n"
	"\n"
	"[NALASR]\n"
	"Name=Laser\n";

char const _Plain_Saved[] =
	"[General]\r\n"
	"Crate=10\r\n"
	"TiberiumGrowth=0.02\r\n"
	"\r\n"
	"[NALASR]\r\n"
	"Name=Laser\r\n";


// A file laid out by hand, which a load that keeps comments has to write back exactly.
char const _Commented[] =
	"; Header comment\r\n"
	"\r\n"
	"[General]\r\n"
	"; About crates\r\n"
	"Crate=10 ; per map\r\n"
	"TiberiumGrowth  = 0.02\r\n"
	";\r\n"
	"\r\n"
	"[NALASR]\r\n"
	"Name=Laser\r\n"
	"; trailing block\r\n";

char const _Commented_Canonical[] =
	"[General]\r\n"
	"Crate=10\r\n"
	"TiberiumGrowth=0.02\r\n"
	"\r\n"
	"[NALASR]\r\n"
	"Name=Laser\r\n";


std::string Numbered_Section(char const * section, int count)
{
	std::string text = "[";
	text += section;
	text += "]\n";
	for (int index = 0; index < count; index++) {
		text += std::to_string(index);
		text += "=value\n";
	}
	return(text);
}


double Seconds_Since(std::chrono::steady_clock::time_point start)
{
	return(std::chrono::duration<double>(std::chrono::steady_clock::now() - start).count());
}


// A 2000 character value: 250 comma separated names, longer than any fixed line buffer.
std::string Long_Value(void)
{
	std::string value;
	for (int index = 0; index < 250; index++) {
		if (index > 0) value += ',';
		char name[16];
		std::snprintf(name, sizeof(name), "N%06d", index);
		value += name;
	}
	value += 'X';
	return(value);
}


std::string Long_Source(void)
{
	return("[Long]\r\nList=" + Long_Value() + "\r\n");
}

}


int main(void)
{
	std::printf("OpenTS INI contract\n\n");

	Debug_Init();

	{
		INIClass ini;
		Read(ini, _Repeated_Entry);

		Check(ini.Entry_Count("NALASR") == 4, "a repeated key is stored once");
		Check(Value_Is(ini, "NALASR", "Turret", "no"), "the later value of a repeated key wins");
		Check(Value_Is(ini, "NALASR", "Name", "Laser"), "the keys around a repeat are untouched");
		Check(Entry_Is(ini, "NALASR", 0, "Name") && Entry_Is(ini, "NALASR", 1, "Primary")
			&& Entry_Is(ini, "NALASR", 2, "Turret") && Entry_Is(ini, "NALASR", 3, "TurretAnim"),
			"a repeated key keeps the place it had reached when it repeated");
	}

	{
		INIClass ini;
		Read(ini, _Repeated_Section);

		Check(ini.Section_Count() == 1, "a repeated section header is stored once");
		Check(ini.Entry_Count("General") == 3, "both blocks of a repeated section are readable");
		Check(Value_Is(ini, "General", "Crate", "10"), "the first block's keys survive");
		Check(Value_Is(ini, "General", "BridgeVoxelMax", "10"), "the second block's keys survive");
		Check(Value_Is(ini, "General", "TiberiumGrowth", "0.09"),
			"a key repeated across the blocks resolves to the later value");
		Check(Entry_Is(ini, "General", 0, "Crate") && Entry_Is(ini, "General", 1, "TiberiumGrowth")
			&& Entry_Is(ini, "General", 2, "BridgeVoxelMax"),
			"the blocks of a repeated section keep their order");
	}

	{
		INIClass ini;
		Read(ini, _Plain);

		Check(ini.Section_Count() == 2, "an ordinary file still reads every section");
		Check(ini.Entry_Count("General") == 2, "an ordinary file still reads every key");
		Check(Value_Is(ini, "General", "TiberiumGrowth", "0.02"), "an ordinary file reads its values");
		Check(Save_Bytes(ini) == _Plain_Saved, "an ordinary file saves as the canonical byte stream");
	}

	{
		// Reading a second file into a populated database is how the rules layers stack, so
		// its repeats are overrides, not mistakes, and they are not reported.
		int repeats = Log_Lines("repeats");
		int appears = Log_Lines("appears again");

		INIClass ini;
		Read(ini, "[General]\nCrate=10\n");
		Read(ini, "[General]\nCrate=20\n");
		Check(Value_Is(ini, "General", "Crate", "20"), "a later file overrides an earlier one");
		Check(Log_Lines("repeats") == repeats && Log_Lines("appears again") == appears,
			"an override from a later file is not reported");

		ini.Put_String("General", "Crate", "30");
		Check(Log_Lines("repeats") == repeats, "a value the engine stores is not reported");

		int crates = Log_Lines("INI: [General] repeats Crate");
		INIClass within;
		Read(within, "[General]\nCrate=10\nCrate=20\n");
		Check(Log_Lines("INI: [General] repeats Crate") == crates + 1, "a key repeated within one file is reported once");

		int headers = Log_Lines("INI: [A] appears again");
		INIClass twice;
		Read(twice, "[A]\nx=1\n[B]\ny=1\n[A]\nz=1\n");
		Check(Log_Lines("INI: [A] appears again") == headers + 1, "a header repeated within one file is reported once");
		Check(twice.Section_Count() == 2 && twice.Entry_Count("A") == 2, "the repeated header's keys join the earlier block");

		INIClass named;
		MemoryFileClass file("RULES.INI", "[General]\nCrate=10\nCrate=20\n");
		named.Load(file);
		Check(Log_Lines("INI: RULES.INI [General] repeats Crate") == 1, "a report names the file when the load had one");
	}

	{
		INIClass ini;
		Read(ini, _Commented, true);
		Check(Save_Bytes(ini) == _Commented, "a load that keeps comments saves the file back exactly");

		INIClass plain;
		Read(plain, _Commented);
		Check(Save_Bytes(plain) == _Commented_Canonical, "a load without comments saves the canonical form");

		INIClass comments;
		BufferStraw straw("; only\r\n", 8);
		Check(!comments.Load(straw), "a file of comments alone does not load without keeping them");
		BufferStraw again("; only\r\n", 8);
		Check(comments.Load(again, true) && Save_Bytes(comments) == "; only\r\n",
			"a file of comments alone loads and saves when they are kept");
	}

	{
		INIClass ini;
		Read(ini, "[S]\na=1\n");
		char const * first = ini.Get_Entry("S", 0);

		char name[16];
		for (int index = 0; index < 10000; index++) {
			std::snprintf(name, sizeof(name), "k%d", index);
			ini.Put_String("S", name, "v");
		}
		Check(first == ini.Get_Entry("S", 0) && std::strcmp(first, "a") == 0,
			"an entry name pointer survives ten thousand additions");

		ini.Put_String("S", "a", "2");
		Check(first == ini.Get_Entry("S", 10000) && Value_Is(ini, "S", "a", "2"),
			"an entry name pointer survives the entry being rewritten and moved");
	}

	{
		INIClass ini;
		Read(ini, "[General]\nCrate=10\n[Digest]\n1=abc\n2=def\n");
		Check(!ini.Is_Present("General", "crate") && !ini.Is_Present("general", "Crate"),
			"section and key names are case sensitive");
		Check(ini.Entry_Count("Digest") == 2 && Entry_Is(ini, "Digest", 0, "1") && Entry_Is(ini, "Digest", 1, "2"),
			"a key that is only digits is a key");

		unsigned char blob[100];
		for (int index = 0; index < (int)sizeof(blob); index++) blob[index] = (unsigned char)(index * 7);
		INIClass block;
		block.Put_UUBlock("Blob", blob, sizeof(blob));
		unsigned char back[100] = {0};
		Check(block.Get_UUBlock("Blob", back, sizeof(back)) == sizeof(blob) && std::memcmp(blob, back, sizeof(blob)) == 0,
			"a binary block round trips through its numbered lines");
	}

	{
		// Two names the old CRC-keyed index could not tell apart.
		char const * first = "VS6SKKTV";
		char const * second = "G671OCJJ";
		Check(CRCEngine()(first, 8) == CRCEngine()(second, 8), "the collision pair still collides under CRCEngine");

		INIClass ini;
		Read(ini, "[X]\nVS6SKKTV=1\nG671OCJJ=2\n");
		Check(ini.Entry_Count("X") == 2 && Value_Is(ini, "X", first, "1") && Value_Is(ini, "X", second, "2"),
			"two names with the same CRC are two keys");
	}

	{
		int before = Log_Lines("using the default");

		INIClass ini;
		Read(ini, "[V]\nF=abc\nP=50%\nV=1.5\nN=-0.75\nP2=0,90\nP3=0,90\nP3b=x,1,2\nP3c=1, 2, 3\nP3d=1 ,2 ,3\nR=10,20\nE=2.5e1\n");

		Check(ini.Get_Float("V", "F", 7.0) == 7.0, "a value that is not a number reads as the default");
		Check(ini.Get_Float("V", "P", 0.0) == 0.5, "a percent sign divides a number by one hundred");
		Check(ini.Get_Float("V", "V", 0.0) == 1.5 && ini.Get_Float("V", "N", 0.0) == -0.75 && ini.Get_Float("V", "E", 0.0) == 25.0,
			"decimal, negative and exponent forms read as before");
		Check(ini.Get_Float("V", "Absent", 3.0) == 3.0, "an absent number reads as the default");

		TPoint2D<int> const two = ini.Get_Point("V", "P2", TPoint2D<int>(9, 9));
		Check(two.X == 0 && two.Y == 90, "two numbers read as a two dimensional point");

		TPoint3D<int> const fallback(9, 9, 9);
		TPoint3D<int> short3 = ini.Get_Point("V", "P3", fallback);
		Check(short3.X == 9 && short3.Y == 9 && short3.Z == 9, "two numbers where three are needed read as the default");
		TPoint3D<int> bad = ini.Get_Point("V", "P3b", fallback);
		Check(bad.X == 9 && bad.Y == 9 && bad.Z == 9, "a non-number in a triplet reads as the default");
		TPoint3D<int> spaced = ini.Get_Point("V", "P3c", fallback);
		TPoint3D<int> spaced2 = ini.Get_Point("V", "P3d", fallback);
		Check(spaced.X == 1 && spaced.Y == 2 && spaced.Z == 3 && spaced2.X == 1 && spaced2.Y == 2 && spaced2.Z == 3,
			"spaces around the commas of a triplet are allowed");
		TPoint3D<int> absent = ini.Get_Point("V", "Absent", fallback);
		Check(absent.X == 9 && absent.Y == 9 && absent.Z == 9, "an absent point reads as the default");

		Rect const rect = ini.Get_Rect("V", "R", Rect(1, 2, 3, 4));
		Check(rect.X == 1 && rect.Y == 2 && rect.Width == 3 && rect.Height == 4,
			"a short rectangle reads as the whole default");

		Check(Log_Lines("using the default") == before + 4, "each malformed value is reported once");

		// The readers moved from sscanf to strtol and strtof; the C runtime has to agree.
		bool agree = true;
		char const * forms[] = {"0", "10", "-3", "0.02", "1.5", "-0.75", "100%", "  7", "2.5e1"};
		for (char const * form : forms) {
			float scanned = 0.0f;
			float converted = std::strtof(form, NULL);
			std::sscanf(form, "%f", &scanned);
			if (std::memcmp(&scanned, &converted, sizeof(float)) != 0) agree = false;

			int scannedint = 0;
			int convertedint = (int)std::strtol(form, NULL, 10);
			if (std::sscanf(form, "%d", &scannedint) == 1 && scannedint != convertedint) agree = false;
		}
		Check(agree, "strtof and strtol agree with sscanf on the forms rules files use");
	}

	{
		INIClass ini;
		Read(ini, "[A]\n[B]\nk=1\n");
		Check(ini.Section_Count() == 1 && ini.Section_Present("B"), "a section with no keys is not stored");

		INIClass kept;
		Read(kept, "[A]\n[B]\nk=1\n", true);
		Check(kept.Section_Count() == 2 && Save_Bytes(kept) == "[A]\r\n\r\n[B]\r\nk=1\r\n",
			"a section with no keys is kept with its comments");
	}

	{
		INIClass ini;
		Read(ini, "[S]\nk=v");
		Check(Value_Is(ini, "S", "k", "v"), "a last line with no newline is read");

		INIClass spaced;
		Read(spaced, "[ General ]\nk=1\n");
		Check(spaced.Section_Present(" General ") && !spaced.Section_Present("General"),
			"a section name keeps the spaces inside its brackets");
	}

	{
		INIClass ini;
		Read(ini, "[S]\na=1\nb=2\n");
		Read(ini, "[S]\na=3\nc=4\n");
		Check(Entry_Is(ini, "S", 0, "b") && Entry_Is(ini, "S", 1, "a") && Entry_Is(ini, "S", 2, "c") && Value_Is(ini, "S", "a", "3"),
			"an override from a later file moves the key to the end");
	}

	{
		std::string const big = Numbered_Section("IsoMapPack5", 20000);
		auto start = std::chrono::steady_clock::now();
		INIClass ini;
		Read(ini, big.c_str());
		double const loaded = Seconds_Since(start);

		start = std::chrono::steady_clock::now();
		int found = 0;
		for (int index = 0; index < 20000; index++) {
			if (ini.Get_Entry("IsoMapPack5", index) != NULL) found++;
		}
		double const swept = Seconds_Since(start);

		std::string many;
		for (int section = 0; section < 5000; section++) {
			many += Numbered_Section(("S" + std::to_string(section)).c_str(), 20);
		}
		start = std::chrono::steady_clock::now();
		INIClass wide;
		Read(wide, many.c_str());
		double const widened = Seconds_Since(start);

		std::printf("  20000 entries loaded in %.3f s, swept in %.3f s; 5000 sections loaded in %.3f s\n", loaded, swept, widened);
		Check(ini.Entry_Count("IsoMapPack5") == 20000 && found == 20000, "a twenty thousand line section reads whole");
		Check(loaded < 2.0, "a twenty thousand line section loads in under two seconds");
		Check(swept < 1.0, "twenty thousand positional reads take under a second");
		Check(wide.Section_Count() == 5000 && widened < 2.0, "five thousand sections load in under two seconds");
	}

	{
		std::string const value = Long_Value();
		std::string const source = Long_Source();
		INIClass ini;
		Read(ini, source.c_str());

		char buffer[4096];
		int length = ini.Get_String("Long", "List", "", buffer, sizeof(buffer));
		Check(length == 2000 && value == buffer, "a two thousand character line is read whole");
		Check(Save_Bytes(ini) == source, "a two thousand character line saves back exactly");

		INIClass merged;
		Read(merged, "[Other]\nk=v\n");
		Read(merged, source.c_str());
		Check(merged.Get_String("Long", "List", "", buffer, sizeof(buffer)) == 2000, "a merging load keeps a two thousand character line");

		std::string const tail = "[S]\nk=" + std::string(1000, 'v');
		INIClass last;
		Read(last, tail.c_str());
		Check(last.Get_String("S", "k", "", buffer, sizeof(buffer)) == 1000, "a thousand character last line with no newline is read whole");
	}

	{
		std::string const value = Long_Value();
		INIClass ini;
		Read(ini, Long_Source().c_str());

		int before = Log_Lines("[Long] List is 2000 characters");
		char narrow[64];
		int length = ini.Get_String("Long", "List", "", narrow, sizeof(narrow));
		Check(length == 63 && narrow[63] == '\0' && std::memcmp(narrow, value.c_str(), 63) == 0,
			"a fixed buffer still receives as much of a long value as it holds");
		ini.Get_String("Long", "List", "", narrow, sizeof(narrow));
		Check(Log_Lines("[Long] List is 2000 characters") == before + 1, "a value cut short by a fixed buffer is reported once");

		Check(ini.Get_String("Long", "List") == value, "a string read returns a long value whole");
		Check(ini.Get_String("Long", "Missing", "  def  ") == "def", "a string read returns the trimmed default when the key is absent");
		Check(ini.Get_String("Long", "Missing", NULL).empty(), "a string read returns nothing for a NULL default");
		Check(ini.Get_String("Long", "List", "other") == value, "a string read ignores the default when the key is present");

		char field[16] = "keep";
		int kept = ini.Get_String("Long", "Missing", field, field, sizeof(field));
		Check(kept == 4 && std::strcmp(field, "keep") == 0, "reading into a field that is its own default leaves it alone");

		std::string tokens = ini.Get_String("Long", "List");
		int count = 0;
		for (char * token = std::strtok(tokens.data(), ","); token != NULL; token = std::strtok(NULL, ",")) count++;
		Check(count == 250, "a long list tokenizes to every name it holds");
	}

	Failures += Test_UUBlock();

	std::printf("\n%s\n", Failures == 0 ? "PASSED" : "FAILED");
	return(Failures == 0 ? 0 : 1);
}
