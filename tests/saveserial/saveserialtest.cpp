// Fails when the save layout or the game checksum may have changed, so that a developer raises
// the serial in ExpectedGameVersion (code/saveload.cpp). It hashes the text of every function
// definition under code/ whose parameter list uses SaveStreamClass, or whose name contains
// Serialize or is Compute_CRC, Compute_Game_CRC, Object_CRCs, Add_CRC, Put_All or Get_All,
// and of the SERIALIZE_BIT macro, and compares the hashes with functions.txt in this folder.
// SaveFileClass::Serialize_Fields is left out.
//
// Comments, line endings and whitespace between tokens do not count toward a hash. The case
// reads the source tree only, so it needs no game data and no build of the engine. A member
// whose type changes in a header changes the layout without changing any hashed text.

#include "doctest/doctest.h"

#include <cstdint>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <map>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

namespace fs = std::filesystem;

namespace {

bool Is_Ident(char c)
{
	return((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '_');
}


bool Is_Space(char c)
{
	return(c == ' ' || c == '\t' || c == '\r' || c == '\n' || c == '\f' || c == '\v');
}


// The length of the identifier that ends just before pos.
std::size_t Ident_Before(std::string_view text, std::size_t pos)
{
	std::size_t start = pos;
	while (start > 0 && Is_Ident(text[start - 1])) {
		start--;
	}
	return(pos - start);
}


// The position just past the string, character or raw string literal that starts at pos, or pos
// itself when no literal starts there.
std::size_t Skip_Literal(std::string_view text, std::size_t pos)
{
	char const c = text[pos];
	if (c != '"' && c != '\'') {
		return(pos);
	}

	std::size_t const prefix = Ident_Before(text, pos);
	std::string_view const before = text.substr(pos - prefix, prefix);

	// A quote after a digit or a name other than an encoding prefix is a digit separator.
	if (c == '\'' && prefix > 0 && before != "L" && before != "u" && before != "U" && before != "u8") {
		return(pos);
	}

	if (c == '"' && (before == "R" || before == "LR" || before == "uR" || before == "UR" || before == "u8R")) {
		std::size_t const open = text.find('(', pos);
		if (open == std::string_view::npos) {
			return(text.size());
		}
		std::string const close = ")" + std::string(text.substr(pos + 1, open - pos - 1)) + "\"";
		std::size_t const end = text.find(close, open);
		return(end == std::string_view::npos ? text.size() : end + close.size());
	}

	std::size_t index = pos + 1;
	while (index < text.size() && text[index] != c && text[index] != '\n') {
		index += (text[index] == '\\') ? 2 : 1;
	}
	return(index < text.size() ? index + 1 : text.size());
}


// The text with each comment replaced by one space and every literal kept as it is.
std::string Strip_Comments(std::string_view text)
{
	std::string out;
	out.reserve(text.size());

	std::size_t index = 0;
	while (index < text.size()) {
		std::size_t const end = Skip_Literal(text, index);
		if (end != index) {
			out.append(text.substr(index, end - index));
			index = end;
			continue;
		}
		if (text.substr(index, 2) == "//") {
			while (index < text.size() && text[index] != '\n') {
				index++;
			}
			out += ' ';
			continue;
		}
		if (text.substr(index, 2) == "/*") {
			std::size_t const close = text.find("*/", index + 2);
			index = (close == std::string_view::npos) ? text.size() : close + 2;
			out += ' ';
			continue;
		}
		out += text[index++];
	}
	return(out);
}


// The text with whitespace removed, except one space where two identifier characters would
// otherwise meet. Literals are kept as they are.
std::string Normalize(std::string_view text)
{
	std::string out;
	bool pending_space = false;

	std::size_t index = 0;
	while (index < text.size()) {
		if (Is_Space(text[index])) {
			pending_space = true;
			index++;
			continue;
		}
		if (pending_space && !out.empty() && Is_Ident(out.back()) && Is_Ident(text[index])) {
			out += ' ';
		}
		pending_space = false;

		std::size_t const end = Skip_Literal(text, index);
		if (end != index) {
			out.append(text.substr(index, end - index));
			index = end;
			continue;
		}
		out += text[index++];
	}
	return(out);
}


// The position of the bracket that closes the one at pos, or npos when the text ends first.
std::size_t Match_Bracket(std::string_view text, std::size_t pos, char open, char close)
{
	int depth = 0;
	std::size_t index = pos;
	while (index < text.size()) {
		std::size_t const end = Skip_Literal(text, index);
		if (end != index) {
			index = end;
			continue;
		}
		if (text[index] == open) {
			depth++;
		} else if (text[index] == close) {
			if (--depth == 0) {
				return(index);
			}
		}
		index++;
	}
	return(std::string_view::npos);
}


// The position of the brace that opens the body of a function whose parameter list closes just
// before pos, or npos when the name was a call or a declaration.
std::size_t Body_Start(std::string_view text, std::size_t pos)
{
	int depth = 0;
	std::size_t index = pos;
	while (index < text.size()) {
		std::size_t const end = Skip_Literal(text, index);
		if (end != index) {
			index = end;
			continue;
		}
		char const c = text[index];
		if (c == '(') {
			depth++;
		} else if (c == ')') {
			if (depth == 0) {
				return(std::string_view::npos);
			}
			depth--;
		} else if (depth == 0) {
			if (c == '{') {
				return(index);
			}
			if (c == ';' || c == ',' || c == '}' || c == '=') {
				return(std::string_view::npos);
			}
		}
		index++;
	}
	return(std::string_view::npos);
}


// The position where the qualified name ending in the identifier at pos starts, so that a
// definition outside its class is named with the class.
std::size_t Qualifier_Start(std::string_view text, std::size_t pos)
{
	std::size_t start = pos;
	for (;;) {
		std::size_t index = start;
		while (index > 0 && Is_Space(text[index - 1])) {
			index--;
		}
		if (index < 2 || text.substr(index - 2, 2) != "::") {
			return(start);
		}
		index -= 2;
		while (index > 0 && Is_Space(text[index - 1])) {
			index--;
		}
		if (index > 0 && text[index - 1] == '>') {
			int depth = 0;
			while (index > 0) {
				char const c = text[--index];
				if (c == '>') {
					depth++;
				} else if (c == '<' && --depth == 0) {
					break;
				}
			}
		}
		std::size_t const length = Ident_Before(text, index);
		if (length == 0) {
			return(start);
		}
		start = index - length;
	}
}


bool Is_Tracked_Name(std::string_view name)
{
	// The container format version governs the save's field table, so the serial does not.
	if (name == "Serialize_Fields") {
		return(false);
	}
	return(name.find("Serialize") != std::string_view::npos
		|| name == "Compute_CRC"
		|| name == "Compute_Game_CRC"
		|| name == "Object_CRCs"
		|| name == "Add_CRC"
		|| name == "Put_All"
		|| name == "Get_All");
}


bool Is_Keyword(std::string_view name)
{
	return(name == "if" || name == "while" || name == "for" || name == "switch" || name == "catch"
		|| name == "return" || name == "sizeof" || name == "decltype" || name == "requires"
		|| name == "noexcept" || name == "alignof" || name == "static_assert");
}


// True when the parameter list text contains the identifier SaveStreamClass.
bool Names_Save_Stream(std::string_view params)
{
	std::string_view const word = "SaveStreamClass";
	for (std::size_t found = params.find(word); found != std::string_view::npos; found = params.find(word, found + 1)) {
		std::size_t const end = found + word.size();
		if ((found == 0 || !Is_Ident(params[found - 1])) && (end >= params.size() || !Is_Ident(params[end]))) {
			return(true);
		}
	}
	return(false);
}


// The names of the macros whose definition text counts toward the record.
bool Is_Tracked_Macro(std::string_view name)
{
	return(name == "SERIALIZE_BIT");
}


std::uint64_t Fnv1a(std::string_view text)
{
	std::uint64_t hash = 0xCBF29CE484222325ull;
	for (char const c : text) {
		hash ^= (unsigned char)c;
		hash *= 0x100000001B3ull;
	}
	return(hash);
}


std::string Hex(std::uint64_t value)
{
	char buffer[17];
	std::snprintf(buffer, sizeof(buffer), "%016llx", (unsigned long long)value);
	return(buffer);
}


void Add_Entry(std::map<std::string, std::string> & table, std::string key, std::string_view text)
{
	std::string const base = key;
	for (int copy = 2; table.count(key) != 0; copy++) {
		key = base + " #" + std::to_string(copy);
	}
	table[key] = Hex(Fnv1a(Normalize(text)));
}


// Adds "<path> #define <name>" for each tracked macro, hashing its text with continuation lines.
void Hash_Macros(std::string_view path, std::string_view view, std::map<std::string, std::string> & table)
{
	std::string_view const directive = "#define";
	for (std::size_t found = view.find(directive); found != std::string_view::npos; found = view.find(directive, found + 1)) {
		std::size_t name = found + directive.size();
		while (name < view.size() && (view[name] == ' ' || view[name] == '\t')) {
			name++;
		}
		std::size_t name_end = name;
		while (name_end < view.size() && Is_Ident(view[name_end])) {
			name_end++;
		}
		if (!Is_Tracked_Macro(view.substr(name, name_end - name))) {
			continue;
		}

		std::size_t end = name_end;
		while (end < view.size()) {
			std::size_t const newline = view.find('\n', end);
			if (newline == std::string_view::npos) {
				end = view.size();
				break;
			}
			std::size_t last = newline;
			while (last > end && Is_Space(view[last - 1])) {
				last--;
			}
			end = newline + 1;
			if (last == 0 || view[last - 1] != '\\') {
				break;
			}
		}
		Add_Entry(table, std::string(path) + " #define " + std::string(view.substr(name, name_end - name)), view.substr(found, end - found));
	}
}


// Maps "<path> <qualified name>(<parameters>)" to the hash of each tracked definition in one
// file's text. A definition is tracked when its name is, or when its parameter list uses
// SaveStreamClass.
void Hash_Definitions(std::string_view path, std::string_view source, std::map<std::string, std::string> & table)
{
	std::string const text = Strip_Comments(source);
	std::string_view const view = text;

	Hash_Macros(path, view, table);

	std::size_t index = 0;
	while (index < view.size()) {
		std::size_t const end = Skip_Literal(view, index);
		if (end != index) {
			index = end;
			continue;
		}
		if (!Is_Ident(view[index]) || (index > 0 && Is_Ident(view[index - 1]))) {
			index++;
			continue;
		}

		std::size_t name_end = index;
		while (name_end < view.size() && Is_Ident(view[name_end])) {
			name_end++;
		}
		std::string_view const name = view.substr(index, name_end - index);

		std::size_t paren = name_end;
		while (paren < view.size() && Is_Space(view[paren])) {
			paren++;
		}
		if (paren >= view.size() || view[paren] != '(' || Is_Keyword(name)) {
			index = name_end;
			continue;
		}

		std::size_t const params_end = Match_Bracket(view, paren, '(', ')');
		if (params_end == std::string_view::npos
			|| (!Is_Tracked_Name(name) && !Names_Save_Stream(view.substr(paren, params_end - paren)))) {
			index = name_end;
			continue;
		}

		std::size_t const body = Body_Start(view, params_end + 1);
		std::size_t const body_end = (body == std::string_view::npos) ? body : Match_Bracket(view, body, '{', '}');
		if (body_end == std::string_view::npos) {
			index = name_end;
			continue;
		}

		std::size_t const qualified = Qualifier_Start(view, index);
		Add_Entry(table, std::string(path) + " " + Normalize(view.substr(qualified, params_end + 1 - qualified)),
			view.substr(qualified, body_end + 1 - qualified));

		index = body_end + 1;
	}
}


std::map<std::string, std::string> Hash_Source_Tree(fs::path const & root)
{
	std::map<std::string, std::string> table;
	for (fs::directory_entry const & entry : fs::recursive_directory_iterator(root / "code")) {
		if (!entry.is_regular_file()) {
			continue;
		}
		std::string const extension = entry.path().extension().string();
		if (extension != ".cpp" && extension != ".h" && extension != ".hh" && extension != ".hpp") {
			continue;
		}

		std::ifstream file(entry.path(), std::ios::binary);
		std::ostringstream content;
		content << file.rdbuf();
		std::string const source = content.str();

		Hash_Definitions(fs::relative(entry.path(), root).generic_string(), source, table);
	}
	return(table);
}


std::map<std::string, std::string> Read_Table(fs::path const & path)
{
	std::map<std::string, std::string> table;
	std::ifstream file(path);
	std::string line;
	while (std::getline(file, line)) {
		if (!line.empty() && line.back() == '\r') {
			line.pop_back();
		}
		if (line.empty() || line[0] == '#') {
			continue;
		}
		std::size_t const space = line.find(' ');
		if (space != std::string::npos) {
			table[line.substr(space + 1)] = line.substr(0, space);
		}
	}
	return(table);
}


std::string Format_Table(std::map<std::string, std::string> const & table)
{
	std::string out =
		"# The text hash of each function that defines the save layout or the game checksum.\n"
		"# tests/saveserial compares these lines with the source. When it fails, raise\n"
		"# GameVersionSerial in code/saveload.cpp by one if the save layout or what the\n"
		"# simulation computes changed, then replace this file with the one the test wrote.\n";
	for (auto const & [key, hash] : table) {
		out += hash + " " + key + "\n";
	}
	return(out);
}


std::map<std::string, std::string> Hash_One(std::string_view source)
{
	std::map<std::string, std::string> table;
	Hash_Definitions("x.cpp", source, table);
	return(table);
}

}	// namespace


TEST_CASE("the save and checksum functions match the recorded hashes")
{
	fs::path const root = OPENTS_SOURCE_DIR;
	fs::path const recorded_path = SAVESERIAL_RECORD;
	fs::path const current_path = SAVESERIAL_CURRENT;

	REQUIRE(fs::is_directory(root / "code"));

	std::map<std::string, std::string> const current = Hash_Source_Tree(root);
	std::map<std::string, std::string> const recorded = Read_Table(recorded_path);
	CHECK(current.size() > 200);

	std::string differences;
	int count = 0;
	for (auto const & [key, hash] : current) {
		auto const found = recorded.find(key);
		if (found == recorded.end()) {
			differences += "  added    " + key + "\n";
			count++;
		} else if (found->second != hash) {
			differences += "  changed  " + key + "\n";
			count++;
		}
	}
	for (auto const & [key, hash] : recorded) {
		if (current.count(key) == 0) {
			differences += "  removed  " + key + "\n";
			count++;
		}
	}

	if (count != 0) {
		fs::create_directories(current_path.parent_path());
		std::ofstream(current_path, std::ios::binary) << Format_Table(current);

		FAIL(count, " save or checksum function(s) differ from ", recorded_path.generic_string(), ":\n",
			differences,
			"If this change alters the save layout or what the simulation computes, raise\n"
			"GameVersionSerial in code/saveload.cpp by one in the same commit, so that saves and\n"
			"replays from the earlier build are refused. Then copy\n  ",
			current_path.generic_string(), "\nover\n  ", recorded_path.generic_string(), "\n");
	}
}


TEST_CASE("comments, line endings and whitespace do not change a hash")
{
	auto const plain = Hash_One("void A::Serialize(S & s)\n{\n\ts.Serialize(X);\n}\n");
	REQUIRE(plain.size() == 1);
	CHECK(plain.begin()->first == "x.cpp A::Serialize(S&s)");

	CHECK(Hash_One("void A :: Serialize (S &s) {\r\n  s.Serialize( X ); // why\r\n}\r\n") == plain);
	CHECK(Hash_One("/* old */ void A::Serialize(S & s)\n{\n\t/* a } brace */ s.Serialize(X);\n}\n") == plain);

	CHECK(Hash_One("void A::Serialize(S & s)\n{\n\ts.Serialize(Y);\n}\n") != plain);
	CHECK(Hash_One("void A::Serialize(S & s)\n{\n\ts.Serialize(\"X  Y\");\n}\n")
		!= Hash_One("void A::Serialize(S & s)\n{\n\ts.Serialize(\"X Y\");\n}\n"));
}


TEST_CASE("calls and declarations are not definitions")
{
	CHECK(Hash_One("void Serialize(S & s);\nint x = Serialize(s);\nif (Serialize(s)) {}\n").empty());
	CHECK(Hash_One("void f() { stream.Serialize(a); }\n").empty());
	CHECK(Hash_One("virtual void Compute_CRC(C & crc) const override;\n").empty());
	CHECK(Hash_One("virtual void Serialize(S & s) = 0;\n").empty());

	auto const table = Hash_One(
		"template<class T> void Serialize(T & v) requires (sizeof(T) > 1) { Serialize_Bytes(&v, 4); }\n"
		"void C<int>::Compute_CRC(C & crc) const { char q = '}'; crc(1'000); }\n"
		"static bool Put_All(S & s, int n) { return R\"x(})x\"[0] == '}'; }\n");
	CHECK(table.size() == 3);
	CHECK(table.count("x.cpp Serialize(T&v)") == 1);
	CHECK(table.count("x.cpp C<int>::Compute_CRC(C&crc)") == 1);
	CHECK(table.count("x.cpp Put_All(S&s,int n)") == 1);
}


TEST_CASE("a definition that takes a save stream is tracked whatever its name")
{
	auto const table = Hash_One(
		"bool MouseClass::Save(SaveStreamClass & stream) const { return(true); }\n"
		"void Load_Vector(int n, SaveStreamClass * stream) { }\n"
		"void Other(int n) { }\n"
		"void Wrapper(MySaveStreamClass & s) { }\n"
		"concept C = requires(T & t, SaveStreamClass & s) { t.Save(s); };\n"
		"void Use(SaveStreamClass & s) { if (Check(s)) { } }\n"
		"void SaveFileClass::Serialize_Fields(std::vector<unsigned char> & table) const { }\n");
	CHECK(table.size() == 3);
	CHECK(table.count("x.cpp MouseClass::Save(SaveStreamClass&stream)") == 1);
	CHECK(table.count("x.cpp Load_Vector(int n,SaveStreamClass*stream)") == 1);
	CHECK(table.count("x.cpp Use(SaveStreamClass&s)") == 1);
}


TEST_CASE("the SERIALIZE_BIT macro text is tracked with its continuation lines")
{
	auto const table = Hash_One("#define SERIALIZE_BIT(s, f) \\\n\tdo { \\\n\t\t(s).Serialize(f); \\\n\t} while (false)\n\nint x;\n");
	REQUIRE(table.size() == 1);
	CHECK(table.begin()->first == "x.cpp #define SERIALIZE_BIT");

	auto const changed = Hash_One("#define SERIALIZE_BIT(s, f) \\\n\tdo { \\\n\t\t(s).Serialize(!f); \\\n\t} while (false)\n\nint x;\n");
	CHECK(changed.begin()->second != table.begin()->second);
	CHECK(Hash_One("#define SERIALIZE_BIT(s, f) \\\n\tdo { \\\n\t\t(s).Serialize(f); \\\n\t} while (false)\n\nint y;\n") == table);
}
