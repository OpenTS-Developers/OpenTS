// Exercises the file a saved game is kept in: the field table the load dialog lists from,
// the compressed content block, and every way the reader refuses a file that is not a
// whole, intact save of a version it knows.
//
// Every file it touches it creates itself, in a scratch directory named by the first
// argument or the working directory, so it reads no game data and leaves nothing behind.

#include "savefile.h"

#include <lzo/lzo1x.h>

#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

static int Failures = 0;
static int Checks = 0;
static std::string Scratch;


static void Check(char const * name, bool condition)
{
	Checks++;
	if (condition) return;
	Failures++;
	printf("FAIL %s\n", name);
}


static void Check_Result(char const * name, SaveFileClass::ResultType actual, SaveFileClass::ResultType expected)
{
	Checks++;
	if (actual == expected) return;
	Failures++;
	printf("FAIL %s: got \"%s\", expected \"%s\"\n", name,
		SaveFileClass::Result_Text(actual), SaveFileClass::Result_Text(expected));
}


static std::string Scratch_Path(char const * name)
{
	return(Scratch + "\\" + name);
}


static std::vector<unsigned char> Noise(std::size_t length, unsigned int seed)
{
	std::vector<unsigned char> data(length);
	unsigned int state = seed * 2654435761u + 1u;
	for (std::size_t index = 0; index < length; index++) {
		state = state * 1103515245u + 12345u;
		data[index] = (unsigned char)((state >> 16) & 0xFF);
	}
	return(data);
}


static std::vector<unsigned char> Prose(std::size_t length)
{
	static char const text[] = "The quick brown fox jumps over the lazy dog. ";
	std::vector<unsigned char> data;
	while (data.size() < length) {
		data.push_back((unsigned char)text[data.size() % (sizeof(text) - 1)]);
	}
	return(data);
}


static std::vector<unsigned char> Read_Whole_File(char const * path)
{
	std::vector<unsigned char> data;
	HANDLE const file = CreateFileA(path, GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL, nullptr);
	if (file == INVALID_HANDLE_VALUE) return(data);
	DWORD const size = GetFileSize(file, nullptr);
	if (size != INVALID_FILE_SIZE && size > 0) {
		data.resize(size);
		DWORD got = 0;
		if (!ReadFile(file, data.data(), size, &got, nullptr) || got != size) data.clear();
	}
	CloseHandle(file);
	return(data);
}


static bool Write_Whole_File(char const * path, std::vector<unsigned char> const & data)
{
	HANDLE const file = CreateFileA(path, GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS,
		FILE_ATTRIBUTE_NORMAL, nullptr);
	if (file == INVALID_HANDLE_VALUE) return(false);
	DWORD written = 0;
	bool ok = true;
	if (!data.empty()) {
		ok = WriteFile(file, data.data(), (DWORD)data.size(), &written, nullptr) && written == data.size();
	}
	CloseHandle(file);
	return(ok);
}


static bool File_Exists(char const * path)
{
	HANDLE const file = CreateFileA(path, GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL, nullptr);
	if (file == INVALID_HANDLE_VALUE) return(false);
	CloseHandle(file);
	return(true);
}


enum {
	FIELD_TITLE = 2,
	FIELD_HOUSE = 3,
	FIELD_VERSION = 16,
	FIELD_WHEN = 13,
	FIELD_MISSING = 77,
};


static void Fill(SaveFileClass & save, std::vector<unsigned char> const & content)
{
	FILETIME when;
	when.dwLowDateTime = 0x12345678u;
	when.dwHighDateTime = 0x01D2C3B4u;

	save.Set_String(FIELD_TITLE, "GDI 04: Eviction Notice");
	save.Set_String(FIELD_HOUSE, "GDI");
	save.Set_Int(FIELD_VERSION, 0x00010203);
	save.Set_Time(FIELD_WHEN, when);
	save.Content = content;
}


static void Check_Fields(char const * prefix, SaveFileClass const & save)
{
	char text[64];
	int value = 0;
	FILETIME when = {};

	Check((std::string(prefix) + ": title present").c_str(), save.Get_String(FIELD_TITLE, text, sizeof(text)));
	Check((std::string(prefix) + ": title text").c_str(), strcmp(text, "GDI 04: Eviction Notice") == 0);
	Check((std::string(prefix) + ": house present").c_str(), save.Get_String(FIELD_HOUSE, text, sizeof(text)));
	Check((std::string(prefix) + ": house text").c_str(), strcmp(text, "GDI") == 0);
	Check((std::string(prefix) + ": version present").c_str(), save.Get_Int(FIELD_VERSION, &value));
	Check((std::string(prefix) + ": version value").c_str(), value == 0x00010203);
	Check((std::string(prefix) + ": time present").c_str(), save.Get_Time(FIELD_WHEN, &when));
	Check((std::string(prefix) + ": time value").c_str(),
		when.dwLowDateTime == 0x12345678u && when.dwHighDateTime == 0x01D2C3B4u);
	Check((std::string(prefix) + ": a missing field is absent").c_str(), !save.Get_String(FIELD_MISSING, text, sizeof(text)));
	Check((std::string(prefix) + ": a field is not found under another kind").c_str(), !save.Get_Int(FIELD_TITLE, &value));

	Check((std::string(prefix) + ": a short buffer is clipped").c_str(),
		save.Get_String(FIELD_TITLE, text, 4) && strcmp(text, "GDI") == 0);
}


static void Test_Round_Trip(void)
{
	std::string const path = Scratch_Path("ROUNDTRIP.SAV");
	std::vector<unsigned char> const content = Prose(300000);

	SaveFileClass written;
	Fill(written, content);
	Check_Result("round trip: write", written.Write(path.c_str()), SaveFileClass::RESULT_OK);
	Check("round trip: no temporary file is left behind", !File_Exists((path + ".tmp").c_str()));

	std::vector<unsigned char> const image = Read_Whole_File(path.c_str());
	Check("round trip: the prose was compressed", !image.empty() && image.size() < content.size() / 4);

	SaveFileClass read;
	Check_Result("round trip: read", read.Read(path.c_str()), SaveFileClass::RESULT_OK);
	Check_Fields("round trip", read);
	Check("round trip: content reads back whole", read.Content == content);

	SaveFileClass listed;
	Check_Result("round trip: fields alone", listed.Read_Fields(path.c_str()), SaveFileClass::RESULT_OK);
	Check_Fields("fields alone", listed);
	Check("fields alone: no content is read", listed.Content.empty());
}


static void Test_Cuts(void)
{
	SaveFileClass save;
	save.Set_String(FIELD_TITLE, "ab\xC3\xA9" "cd");

	char text[8];
	Check("cuts: a cut never splits a character", save.Get_String(FIELD_TITLE, text, 4) && strcmp(text, "ab") == 0);
	Check("cuts: a cut after a character keeps it whole", save.Get_String(FIELD_TITLE, text, 5) && strcmp(text, "ab\xC3\xA9") == 0);
	Check("cuts: a buffer that fits keeps everything", save.Get_String(FIELD_TITLE, text, 8) && strcmp(text, "ab\xC3\xA9" "cd") == 0);
}


static void Test_Incompressible(void)
{
	std::string const path = Scratch_Path("NOISE.SAV");
	std::vector<unsigned char> const content = Noise(70000, 7);

	SaveFileClass written;
	Fill(written, content);
	Check_Result("noise: write", written.Write(path.c_str()), SaveFileClass::RESULT_OK);

	std::vector<unsigned char> const image = Read_Whole_File(path.c_str());
	Check("noise: stored as it is when compression does not pay", image.size() >= content.size() + SaveFileClass::HEADER_SIZE);

	SaveFileClass read;
	Check_Result("noise: read", read.Read(path.c_str()), SaveFileClass::RESULT_OK);
	Check("noise: content reads back whole", read.Content == content);
}


static void Test_Empty(void)
{
	std::string const path = Scratch_Path("EMPTY.SAV");

	SaveFileClass written;
	Check_Result("empty: write", written.Write(path.c_str()), SaveFileClass::RESULT_OK);

	std::vector<unsigned char> const image = Read_Whole_File(path.c_str());
	Check("empty: a header alone", image.size() == SaveFileClass::HEADER_SIZE);

	SaveFileClass read;
	Check_Result("empty: read", read.Read(path.c_str()), SaveFileClass::RESULT_OK);
	Check("empty: no content", read.Content.empty());
	char text[8];
	Check("empty: no fields", !read.Get_String(FIELD_TITLE, text, sizeof(text)));
}


static void Test_Overwrite(void)
{
	std::string const path = Scratch_Path("REPLACE.SAV");

	SaveFileClass first;
	Fill(first, Prose(5000));
	first.Set_String(FIELD_TITLE, "the earlier save");
	Check_Result("replace: first write", first.Write(path.c_str()), SaveFileClass::RESULT_OK);

	Check("replace: a stale temporary is planted", Write_Whole_File((path + ".tmp").c_str(), Noise(100, 3)));

	SaveFileClass second;
	Fill(second, Noise(20000, 11));
	second.Set_String(FIELD_TITLE, "the later save");
	Check_Result("replace: second write", second.Write(path.c_str()), SaveFileClass::RESULT_OK);
	Check("replace: the stale temporary is gone", !File_Exists((path + ".tmp").c_str()));

	SaveFileClass read;
	Check_Result("replace: read", read.Read(path.c_str()), SaveFileClass::RESULT_OK);
	char text[64];
	Check("replace: the later save is the one on disk",
		read.Get_String(FIELD_TITLE, text, sizeof(text)) && strcmp(text, "the later save") == 0);
	Check("replace: the later content is the one on disk", read.Content == second.Content);

	SaveFileClass rewritten;
	rewritten.Set_String(FIELD_TITLE, "overwritten field");
	rewritten.Set_String(FIELD_TITLE, "final field");
	Check_Result("replace: field rewrite", rewritten.Write(path.c_str()), SaveFileClass::RESULT_OK);
	Check_Result("replace: field rewrite read", read.Read_Fields(path.c_str()), SaveFileClass::RESULT_OK);
	Check("replace: a field set twice keeps the last value",
		read.Get_String(FIELD_TITLE, text, sizeof(text)) && strcmp(text, "final field") == 0);
}


static void Put_U32(std::vector<unsigned char> & image, std::size_t at, unsigned int value)
{
	image[at] = (unsigned char)(value & 0xFF);
	image[at + 1] = (unsigned char)((value >> 8) & 0xFF);
	image[at + 2] = (unsigned char)((value >> 16) & 0xFF);
	image[at + 3] = (unsigned char)((value >> 24) & 0xFF);
}


static void Test_Limits(void)
{
	std::string const path = Scratch_Path("LIMITS.SAV");

	SaveFileClass kept;
	Fill(kept, Prose(3000));
	kept.Set_String(FIELD_TITLE, "the save that stays");
	Check_Result("limits: the save that stays", kept.Write(path.c_str()), SaveFileClass::RESULT_OK);

	SaveFileClass wide;
	Fill(wide, Prose(3000));
	wide.Set_String(FIELD_TITLE, std::string(0x10001, 'x').c_str());
	Check_Result("limits: a field beyond its limit is refused", wide.Write(path.c_str()), SaveFileClass::RESULT_TOO_LARGE);

	SaveFileClass many;
	Fill(many, Prose(3000));
	for (int id = 100; id < 117; id++) {
		many.Set_String(id, std::string(0x10000, 'y').c_str());
	}
	Check_Result("limits: a table beyond its limit is refused", many.Write(path.c_str()), SaveFileClass::RESULT_TOO_LARGE);

	SaveFileClass huge;
	huge.Content.resize(0x10000001);
	Check_Result("limits: content beyond its limit is refused", huge.Write(path.c_str()), SaveFileClass::RESULT_TOO_LARGE);

	Check("limits: no temporary is left behind", !File_Exists((path + ".tmp").c_str()));
	SaveFileClass read;
	Check_Result("limits: the earlier save still reads", read.Read(path.c_str()), SaveFileClass::RESULT_OK);
	char text[64];
	Check("limits: the earlier save is the one on disk",
		read.Get_String(FIELD_TITLE, text, sizeof(text)) && strcmp(text, "the save that stays") == 0);
}


// Recomputes the header checksum after a test has changed a header byte on purpose.
static void Reseal_Header(std::vector<unsigned char> & image, unsigned int table)
{
	unsigned int crc = SaveFileClass::Checksum(image.data(), SaveFileClass::HEADER_SIZE - 4);
	crc = SaveFileClass::Checksum(image.data() + SaveFileClass::HEADER_SIZE, table, crc);
	Put_U32(image, 28, crc);
}


// Rebuilds a save image around a field table of the test's own making, with the content
// kept and every checksum made good.
static std::vector<unsigned char> Forge_Table(std::vector<unsigned char> const & image, unsigned int table,
	std::vector<unsigned char> const & newtable)
{
	std::vector<unsigned char> forged(image.begin(), image.begin() + SaveFileClass::HEADER_SIZE);
	forged.insert(forged.end(), newtable.begin(), newtable.end());
	forged.insert(forged.end(), image.begin() + SaveFileClass::HEADER_SIZE + table, image.end());
	Put_U32(forged, 8, (unsigned int)newtable.size());
	Put_U32(forged, 12, SaveFileClass::HEADER_SIZE + (unsigned int)newtable.size());
	Reseal_Header(forged, (unsigned int)newtable.size());
	return(forged);
}


// Replaces the content of a save image with a compressed block of the test's own making,
// declared as expanding to the length given, with every checksum made good.
static std::vector<unsigned char> Forge_Content(std::vector<unsigned char> const & image, unsigned int table,
	std::vector<unsigned char> const & stored, unsigned int expands_to)
{
	std::vector<unsigned char> forged(image.begin(), image.begin() + SaveFileClass::HEADER_SIZE + table);
	forged.insert(forged.end(), stored.begin(), stored.end());
	forged[6] |= 0x01;
	Put_U32(forged, 12, SaveFileClass::HEADER_SIZE + table);
	Put_U32(forged, 16, (unsigned int)stored.size());
	Put_U32(forged, 20, expands_to);
	Put_U32(forged, 24, SaveFileClass::Checksum(stored.data(), (unsigned int)stored.size()));
	Reseal_Header(forged, table);
	return(forged);
}


static void Test_Refusals(void)
{
	SaveFileClass read;

	std::string const missing = Scratch_Path("MISSING.SAV");
	Check_Result("refuse: a missing file", read.Read(missing.c_str()), SaveFileClass::RESULT_MISSING);
	Check_Result("refuse: a missing file's fields", read.Read_Fields(missing.c_str()), SaveFileClass::RESULT_MISSING);

	std::string const plain = Scratch_Path("PLAIN.SAV");
	std::vector<unsigned char> hello = { 'h', 'e', 'l', 'l', 'o', ' ', 'w', 'o', 'r', 'l', 'd' };
	Write_Whole_File(plain.c_str(), hello);
	Check_Result("refuse: a file that is not a save", read.Read(plain.c_str()), SaveFileClass::RESULT_NOT_A_SAVE);
	Check_Result("refuse: its fields", read.Read_Fields(plain.c_str()), SaveFileClass::RESULT_NOT_A_SAVE);

	std::string const good = Scratch_Path("GOOD.SAV");
	SaveFileClass written;
	Fill(written, Prose(40000));
	Check_Result("refuse: the reference save", written.Write(good.c_str()), SaveFileClass::RESULT_OK);
	std::vector<unsigned char> const image = Read_Whole_File(good.c_str());
	Check("refuse: the reference save is readable", !image.empty());

	std::string const damaged = Scratch_Path("DAMAGED.SAV");

	unsigned int const table = (unsigned int)image[8] | ((unsigned int)image[9] << 8)
		| ((unsigned int)image[10] << 16) | ((unsigned int)image[11] << 24);

	std::vector<unsigned char> future = image;
	future[4] = 99;
	future[5] = 0;
	Reseal_Header(future, table);
	Write_Whole_File(damaged.c_str(), future);
	Check_Result("refuse: a later format version", read.Read(damaged.c_str()), SaveFileClass::RESULT_UNSUPPORTED_VERSION);
	Check_Result("refuse: its fields", read.Read_Fields(damaged.c_str()), SaveFileClass::RESULT_UNSUPPORTED_VERSION);

	std::vector<unsigned char> flagged = image;
	flagged[6] |= 0x02;
	Reseal_Header(flagged, table);
	Write_Whole_File(damaged.c_str(), flagged);
	Check_Result("refuse: a header flag this build does not know", read.Read(damaged.c_str()), SaveFileClass::RESULT_UNSUPPORTED_VERSION);
	Check_Result("refuse: its fields", read.Read_Fields(damaged.c_str()), SaveFileClass::RESULT_UNSUPPORTED_VERSION);

	unsigned int const content_offset = (unsigned int)image[12] | ((unsigned int)image[13] << 8)
		| ((unsigned int)image[14] << 16) | ((unsigned int)image[15] << 24);
	unsigned int const content_length = (unsigned int)image[20] | ((unsigned int)image[21] << 8)
		| ((unsigned int)image[22] << 16) | ((unsigned int)image[23] << 24);
	Check("refuse: the reference save is compressed", (image[6] & 0x01) != 0);
	std::vector<unsigned char> const stored(image.begin() + content_offset, image.end());

	// The decompressor does not bounds check, so a block that expands past the buffer sized
	// from its declared length cannot be exercised here without corrupting memory. Only a
	// block that stops short of that length is safe to read back.
	Write_Whole_File(damaged.c_str(), Forge_Content(image, table, stored, content_length + 1));
	Check_Result("refuse: a block that ends before its declared length", read.Read(damaged.c_str()), SaveFileClass::RESULT_CORRUPT);

	Write_Whole_File(damaged.c_str(), Forge_Content(image, table, stored, 0x10000001));
	Check_Result("refuse: a block declared larger than any save", read.Read(damaged.c_str()), SaveFileClass::RESULT_CORRUPT);
	Check_Result("refuse: its fields", read.Read_Fields(damaged.c_str()), SaveFileClass::RESULT_CORRUPT);

	std::vector<unsigned char> const oversized(0x100001, 0);
	Write_Whole_File(damaged.c_str(), Forge_Table(image, table, oversized));
	Check_Result("refuse: a field table longer than any listing", read.Read(damaged.c_str()), SaveFileClass::RESULT_CORRUPT);
	Check_Result("refuse: its fields", read.Read_Fields(damaged.c_str()), SaveFileClass::RESULT_CORRUPT);

	std::vector<unsigned char> gapped = image;
	gapped.insert(gapped.begin() + content_offset, 8, 0);
	Put_U32(gapped, 12, content_offset + 8);
	Reseal_Header(gapped, table);
	Check("refuse: the gapped image is longer", gapped.size() == image.size() + 8);
	Write_Whole_File(damaged.c_str(), gapped);
	Check_Result("refuse: a gap between the table and the content", read.Read(damaged.c_str()), SaveFileClass::RESULT_CORRUPT);
	Check_Result("refuse: its fields", read.Read_Fields(damaged.c_str()), SaveFileClass::RESULT_CORRUPT);

	std::vector<unsigned char> header_hit = image;
	header_hit[9] ^= 0x01;
	Write_Whole_File(damaged.c_str(), header_hit);
	Check_Result("refuse: a header byte flipped", read.Read(damaged.c_str()), SaveFileClass::RESULT_CORRUPT);
	Check_Result("refuse: its fields", read.Read_Fields(damaged.c_str()), SaveFileClass::RESULT_CORRUPT);

	std::vector<unsigned char> table_hit = image;
	table_hit[SaveFileClass::HEADER_SIZE + 10] ^= 0x20;
	Write_Whole_File(damaged.c_str(), table_hit);
	Check_Result("refuse: a field byte flipped", read.Read(damaged.c_str()), SaveFileClass::RESULT_CORRUPT);
	Check_Result("refuse: its fields", read.Read_Fields(damaged.c_str()), SaveFileClass::RESULT_CORRUPT);

	std::vector<unsigned char> content_hit = image;
	content_hit[image.size() - 40] ^= 0x80;
	Write_Whole_File(damaged.c_str(), content_hit);
	Check_Result("refuse: a content byte flipped", read.Read(damaged.c_str()), SaveFileClass::RESULT_CORRUPT);
	Check_Result("refuse: a flipped content byte still lists", read.Read_Fields(damaged.c_str()), SaveFileClass::RESULT_OK);

	std::size_t const cuts[] = { 3, 12, SaveFileClass::HEADER_SIZE - 1, SaveFileClass::HEADER_SIZE + 5,
		SaveFileClass::HEADER_SIZE + table, image.size() / 2, image.size() - 1 };
	for (std::size_t cut : cuts) {
		std::vector<unsigned char> truncated(image.begin(), image.begin() + cut);
		Write_Whole_File(damaged.c_str(), truncated);
		char name[80];
		snprintf(name, sizeof(name), "refuse: a file cut at %u bytes", (unsigned int)cut);
		SaveFileClass::ResultType const result = read.Read(damaged.c_str());
		Check(name, result == SaveFileClass::RESULT_CORRUPT || (cut < 4 && result == SaveFileClass::RESULT_NOT_A_SAVE));
	}

	std::vector<unsigned char> appended = image;
	appended.push_back(0);
	Write_Whole_File(damaged.c_str(), appended);
	Check_Result("refuse: a file with a trailing byte", read.Read(damaged.c_str()), SaveFileClass::RESULT_CORRUPT);
}


int main(int argc, char ** argv)
{
	if (lzo_init() != LZO_E_OK) {
		printf("lzo_init failed\n");
		return(2);
	}

	Scratch = (argc > 1) ? argv[1] : ".";
	CreateDirectoryA(Scratch.c_str(), nullptr);

	Test_Round_Trip();
	Test_Cuts();
	Test_Incompressible();
	Test_Empty();
	Test_Overwrite();
	Test_Limits();
	Test_Refusals();

	char const * const names[] = { "ROUNDTRIP.SAV", "NOISE.SAV", "EMPTY.SAV", "REPLACE.SAV",
		"PLAIN.SAV", "GOOD.SAV", "DAMAGED.SAV" };
	for (char const * name : names) {
		DeleteFileA(Scratch_Path(name).c_str());
	}

	printf("%d checks, %d failures\n", Checks, Failures);
	return(Failures == 0 ? 0 : 1);
}
