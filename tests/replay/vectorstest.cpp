/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#include "replay/replayreader.h"
#include "replay/replayseal.h"

#include "fixtures.h"
#include "replaytest.h"
#include "sha256.h"

#include "doctest/doctest.h"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <map>
#include <memory>
#include <set>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

using namespace Replay;

// These cases read the vectors that the Python reference reader generated, so the two
// implementations are checked against each other.

namespace {

// The subset of JSON that vectors.json uses.
struct Json {
	enum class Kind { NUL, BOOL, NUMBER, STRING, ARRAY, OBJECT };

	Kind Type = Kind::NUL;
	bool Flag = false;
	std::int64_t Number = 0;
	std::string Text;
	std::vector<Json> Items;
	std::vector<std::pair<std::string, Json>> Members;

	Json const * Find(std::string_view key) const
	{
		for (auto const & member : Members) {
			if (member.first == key) {
				return(&member.second);
			}
		}
		return(nullptr);
	}

	bool Has(std::string_view key) const
	{
		Json const * value = Find(key);
		return(value != nullptr && value->Type != Kind::NUL);
	}

	Json const & operator[](std::string_view key) const
	{
		Json const * value = Find(key);
		if (value == nullptr) {
			throw std::runtime_error("vectors.json has no key " + std::string(key));
		}
		return(*value);
	}
};


class JsonParser
{
	public:
		explicit JsonParser(std::string_view text) : In(text), At(0) {}

		Json Parse(void)
		{
			Json value = Value();
			Space();
			if (At != In.size()) {
				Fail("trailing text");
			}
			return(value);
		}

	private:
		[[noreturn]] void Fail(char const * what) const
		{
			throw std::runtime_error(std::string("vectors.json: ") + what + " at offset " + std::to_string(At));
		}

		void Space(void)
		{
			while (At < In.size() && (In[At] == ' ' || In[At] == '\t' || In[At] == '\n' || In[At] == '\r')) {
				At++;
			}
		}

		bool Take(std::string_view word)
		{
			if (In.substr(At, word.size()) == word) {
				At += word.size();
				return(true);
			}
			return(false);
		}

		Json Value(void)
		{
			Space();
			if (At >= In.size()) {
				Fail("unexpected end");
			}
			Json value;
			char const c = In[At];
			if (c == '{') {
				value.Type = Json::Kind::OBJECT;
				At++;
				Space();
				if (Take("}")) {
					return(value);
				}
				for (;;) {
					Space();
					std::string key = String();
					Space();
					if (!Take(":")) {
						Fail("expected a colon");
					}
					value.Members.emplace_back(std::move(key), Value());
					Space();
					if (Take("}")) {
						return(value);
					}
					if (!Take(",")) {
						Fail("expected a comma");
					}
				}
			}
			if (c == '[') {
				value.Type = Json::Kind::ARRAY;
				At++;
				Space();
				if (Take("]")) {
					return(value);
				}
				for (;;) {
					value.Items.push_back(Value());
					Space();
					if (Take("]")) {
						return(value);
					}
					if (!Take(",")) {
						Fail("expected a comma");
					}
				}
			}
			if (c == '"') {
				value.Type = Json::Kind::STRING;
				value.Text = String();
				return(value);
			}
			if (Take("true")) {
				value.Type = Json::Kind::BOOL;
				value.Flag = true;
				return(value);
			}
			if (Take("false")) {
				value.Type = Json::Kind::BOOL;
				return(value);
			}
			if (Take("null")) {
				return(value);
			}

			// Every number in the manifest is an integer.
			value.Type = Json::Kind::NUMBER;
			bool const negative = Take("-");
			if (At >= In.size() || In[At] < '0' || In[At] > '9') {
				Fail("expected a value");
			}
			while (At < In.size() && In[At] >= '0' && In[At] <= '9') {
				value.Number = value.Number * 10 + (In[At++] - '0');
			}
			if (At < In.size() && (In[At] == '.' || In[At] == 'e' || In[At] == 'E')) {
				Fail("a number that is not an integer");
			}
			if (negative) {
				value.Number = -value.Number;
			}
			return(value);
		}

		unsigned Hex4(void)
		{
			if (At + 4 > In.size()) {
				Fail("a short \\u escape");
			}
			unsigned code = 0;
			for (int i = 0; i < 4; i++) {
				char const c = In[At++];
				code <<= 4;
				if (c >= '0' && c <= '9') {
					code |= unsigned(c - '0');
				} else if (c >= 'a' && c <= 'f') {
					code |= unsigned(c - 'a' + 10);
				} else if (c >= 'A' && c <= 'F') {
					code |= unsigned(c - 'A' + 10);
				} else {
					Fail("a bad \\u escape");
				}
			}
			return(code);
		}

		static void Put_Utf8(std::string & out, unsigned code)
		{
			if (code < 0x80) {
				out += char(code);
			} else if (code < 0x800) {
				out += char(0xC0 | (code >> 6));
				out += char(0x80 | (code & 0x3F));
			} else if (code < 0x10000) {
				out += char(0xE0 | (code >> 12));
				out += char(0x80 | ((code >> 6) & 0x3F));
				out += char(0x80 | (code & 0x3F));
			} else {
				out += char(0xF0 | (code >> 18));
				out += char(0x80 | ((code >> 12) & 0x3F));
				out += char(0x80 | ((code >> 6) & 0x3F));
				out += char(0x80 | (code & 0x3F));
			}
		}

		std::string String(void)
		{
			if (!Take("\"")) {
				Fail("expected a string");
			}
			std::string text;
			for (;;) {
				if (At >= In.size()) {
					Fail("an unterminated string");
				}
				char const c = In[At++];
				if (c == '"') {
					return(text);
				}
				if (c != '\\') {
					text += c;
					continue;
				}
				if (At >= In.size()) {
					Fail("an unterminated escape");
				}
				switch (In[At++]) {
					case '"': text += '"'; break;
					case '\\': text += '\\'; break;
					case '/': text += '/'; break;
					case 'b': text += '\b'; break;
					case 'f': text += '\f'; break;
					case 'n': text += '\n'; break;
					case 'r': text += '\r'; break;
					case 't': text += '\t'; break;
					case 'u': {
						unsigned code = Hex4();
						if (code >= 0xD800 && code < 0xDC00 && Take("\\u")) {
							unsigned const low = Hex4();
							code = 0x10000 + ((code - 0xD800) << 10) + (low - 0xDC00);
						}
						Put_Utf8(text, code);
						break;
					}
					default:
						Fail("an unknown escape");
				}
			}
		}

		std::string_view In;
		std::size_t At;
};


std::filesystem::path Vectors_Dir(void)
{
	return(std::filesystem::path(REPLAY_VECTORS_DIR));
}


ByteVector Load(std::filesystem::path const & path)
{
	std::ifstream in(path, std::ios::binary);
	REQUIRE_MESSAGE(in.good(), "cannot open ", path.string());
	return(ByteVector((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>()));
}


Json const & Manifest(void)
{
	static std::unique_ptr<Json> manifest;
	if (!manifest) {
		ByteVector const text = Load(Vectors_Dir() / "vectors.json");
		JsonParser parser(std::string_view(reinterpret_cast<char const *>(text.data()), text.size()));
		manifest = std::make_unique<Json>(parser.Parse());
	}
	return(*manifest);
}


std::string Hex_Text(std::string_view text)
{
	return(Hex(std::span(reinterpret_cast<std::uint8_t const *>(text.data()), text.size())));
}


// What the layer makes of a whole file, and what it reads from it before the first failure.
struct Judgement {
	std::string Verdict;   // complete, cut short, malformed, not a CNCR file, or not verified
	std::string Reason;
	std::optional<Digest> S0;
	RecordingInfo Info;
	Launch Start;
	std::vector<std::pair<std::uint32_t, Frame>> Records;
	std::map<std::uint32_t, std::vector<Seal>> Closed;   // the computed seals each record should have
	std::vector<Seal> Chain;                             // every computed seal, the final seal last
	bool HasSummary = false;
	RecordingSummary Summary;
	Bytes StatBody;
	std::vector<Bytes> CheckpointBodies;
};


char const * Read_Name(ReadResult result)
{
	switch (result) {
		case ReadResult::OK: return("OK");
		case ReadResult::NOT_REPLAY: return("NOT_REPLAY");
		case ReadResult::VERSION: return("VERSION");
		case ReadResult::MALFORMED: return("MALFORMED");
		case ReadResult::LIMIT: return("LIMIT");
		case ReadResult::MISSING: return("MISSING");
		case ReadResult::IO: return("IO");
		case ReadResult::MEMORY: return("MEMORY");
	}
	return("?");
}


char const * Status_Verdict(FileStatus status)
{
	switch (status) {
		case FileStatus::COMPLETE: return("complete");
		case FileStatus::INCOMPLETE: return("cut short");
		case FileStatus::NOT_VERIFIED: return("not verified");
		case FileStatus::NOT_REPLAY: return("not a CNCR file");

		// The reader's default limits are the format's, so a file past one is malformed.
		case FileStatus::LIMIT:
		case FileStatus::MISMATCH:
		case FileStatus::MALFORMED: return("malformed");
	}
	return("?");
}


// Takes the verdict from Verify_File, and then reads the file's records and computes its chain for
// the checks of the manifest's values, stopping at the first failure.
Judgement Judge(ByteVector const & file)
{
	Judgement verdict;
	InputFile source(file);
	Reader reader(source);
	FileReport const report = Verify_File(reader);
	verdict.Verdict = Status_Verdict(report.Status);
	verdict.Reason = Read_Name(report.Error);
	if (report.Status == FileStatus::MISMATCH && report.Seals && report.Seals->FirstMismatch) {
		verdict.Reason = "MISMATCH through " + std::to_string(*report.Seals->FirstMismatch);
	}

	if (reader.Open() != ReadResult::OK
		|| reader.Read_Info(verdict.Info) != ReadResult::OK
		|| reader.Read_Launch(verdict.Start) != ReadResult::OK) {
		return(verdict);
	}

	Bytes launch;
	Bytes init;
	if (reader.Read_Body(*reader.Find(TAG_LNCH), launch) != ReadResult::OK) {
		return(verdict);
	}
	// Open requires INIT, so it is missing here only when its codec is unknown.
	SectionInfo const * init_section = reader.Find(TAG_INIT);
	if (init_section != nullptr && reader.Read_Body(*init_section, init) != ReadResult::OK) {
		return(verdict);
	}
	std::optional<SealChain> chain;
	if (init_section != nullptr) {
		verdict.S0 = Seal_Start(verdict.Info.SealVersion, verdict.Info.Engine, verdict.Info.DataVersion, launch, init);
		chain.emplace(*verdict.S0);
	}

	FrameReader frames(reader);
	std::uint32_t frame = 0;
	Frame record;
	FrameStep step;
	while ((step = frames.Next(frame, record)) == FrameStep::RECORD) {
		// A record with fewer seals than the intervals that end before it is malformed, and a
		// forged frame number could make the chain millions of seals long, so it stops there.
		if (chain && chain->Intervals_Before(frame) > record.Seals.size()) {
			chain.reset();
		}
		if (chain) {
			std::vector<Seal> closed;
			chain->Add(frame, record, closed);
			verdict.Chain.insert(verdict.Chain.end(), closed.begin(), closed.end());
			verdict.Closed[frame] = closed;
		}
		verdict.Records.emplace_back(frame, record);
	}
	if (step == FrameStep::FAILED) {
		return(verdict);
	}

	if (reader.Complete()) {
		if (reader.Read_Summary(verdict.Summary) != ReadResult::OK) {
			return(verdict);
		}
		verdict.HasSummary = true;
		if (chain && verdict.Summary.LastFrame
			&& verdict.Chain.size() + chain->Intervals_Before(*verdict.Summary.LastFrame) < verdict.Summary.Seals.size() + 1) {
			chain->Finish(*verdict.Summary.LastFrame, verdict.Chain);
		}
	}
	if (SectionInfo const * stat_section = reader.Find(TAG_STAT); stat_section != nullptr) {
		if (reader.Read_Body(*stat_section, verdict.StatBody) != ReadResult::OK) {
			return(verdict);
		}
	}
	for (SectionInfo const * section : reader.Checkpoints()) {
		Bytes body;
		if (reader.Read_Body(*section, body) != ReadResult::OK) {
			return(verdict);
		}
		verdict.CheckpointBodies.push_back(body);
	}
	return(verdict);
}


// The verdicts behind the exit codes of cncreplay.py verify.
std::string Expected_Verdict(std::int64_t verify_exit)
{
	switch (verify_exit) {
		case 0: return("complete");
		case 1: return("malformed");
		case 2: return("not a CNCR file");
		case 3: return("cut short");
		case 4: return("not verified");
	}
	return("exit " + std::to_string(verify_exit));
}


std::string Seal_List(std::vector<Seal> const & seals)
{
	std::string text;
	for (Seal const & seal : seals) {
		text += std::to_string(seal.Through) + ":" + Hex(seal.Hash) + " ";
	}
	return(text);
}


std::string Seal_List(Json const & seals)
{
	std::string text;
	for (Json const & seal : seals.Items) {
		text += std::to_string(seal["through"].Number) + ":" + seal["hash"].Text + " ";
	}
	return(text);
}


std::vector<std::uint32_t> Throughs(std::vector<Seal> const & seals)
{
	std::vector<std::uint32_t> throughs;
	for (Seal const & seal : seals) {
		throughs.push_back(seal.Through);
	}
	return(throughs);
}


Frame const * Record_At(Judgement const & verdict, std::uint32_t frame)
{
	for (auto const & entry : verdict.Records) {
		if (entry.first == frame) {
			return(&entry.second);
		}
	}
	return(nullptr);
}

}


TEST_CASE("the Python vector computations match the layer")
{
	// Computations of parts of the format this layer does not implement.
	std::set<std::string> const not_computed = {
		"chat_fingerprint", "launch_request", "launch_result",
		"compare_desync", "compare_sealed_payload", "compare_other_game",
	};

	int computed = 0;
	for (Json const & entry : Manifest()["computations"].Items) {
		std::string const name = entry["name"].Text;
		CAPTURE(name);

		if (name == "s0_base" || name == "s0_empty") {
			ByteVector const launch = From_Hex(entry["lnch"].Text);
			ByteVector const init = From_Hex(entry["init"].Text);
			Digest const s0 = Seal_Start(std::uint32_t(entry["seal_version"].Number), entry["engine"].Text,
				std::uint32_t(entry["data_version"].Number), launch, init);
			CHECK(Hex(s0) == entry["s0"].Text);
			if (entry.Has("preimage")) {
				CHECK(Hex(SHA256Class::Digest(From_Hex(entry["preimage"].Text))) == entry["s0"].Text);
				CHECK(Hex(SHA256Class::Digest(launch)) == entry["lnch_sha256"].Text);
				CHECK(Hex(SHA256Class::Digest(init)) == entry["init_sha256"].Text);
			}
			computed++;
		} else if (name == "utf8_replacement") {
			ByteVector const input = From_Hex(entry["input"].Text);
			std::string const cleaned = Clean_Text(std::string_view(reinterpret_cast<char const *>(input.data()), input.size()), 1024);
			CHECK(Hex_Text(cleaned) == entry["output"].Text);
			computed++;
		} else {
			CHECK_MESSAGE(not_computed.count(name) == 1, "a computation this test does not know");
		}
	}
	CHECK(computed == 3);
}


TEST_CASE("every Python vector file gets the manifest's verdict")
{
	std::set<std::string> listed;
	for (Json const & entry : Manifest()["files"].Items) {
		listed.insert(entry["file"].Text);
	}
	std::set<std::string> present;
	for (auto const & item : std::filesystem::directory_iterator(Vectors_Dir())) {
		if (item.path().extension() == ".cncr") {
			present.insert(item.path().filename().string());
		}
	}
	CHECK(present == listed);

	for (Json const & entry : Manifest()["files"].Items) {
		std::string const name = entry["file"].Text;
		ByteVector const file = Load(Vectors_Dir() / name);
		REQUIRE_MESSAGE(Hex(SHA256Class::Digest(file)) == entry["sha256"].Text, name, " is not the file the manifest describes");

		Judgement const verdict = Judge(file);
		std::string const expected = Expected_Verdict(entry["verify_exit"].Number);
		std::string const reason = entry.Has("reason") ? entry["reason"].Text : std::string("valid");
		CHECK_MESSAGE(verdict.Verdict == expected, name, ": the manifest expects ", expected, " (", reason,
			"), C++ says ", verdict.Verdict, " (", verdict.Reason, ")");
	}
}


namespace {

// The records of a file's frame stream as they are stored, with their frame numbers. Only
// the record encoding is checked, so a record the reader refuses for its contents is listed.
std::vector<std::pair<std::uint32_t, Bytes>> Stored_Records(ByteVector const & file)
{
	std::vector<std::pair<std::uint32_t, Bytes>> records;
	InputFile source(file);
	Reader reader(source);
	if (reader.Open() != ReadResult::OK || reader.Find(TAG_FRMS) == nullptr) {
		return(records);
	}
	SectionInfo const & frames = *reader.Find(TAG_FRMS);
	Bytes stored(std::size_t(frames.Stored));
	source.Seek(int(frames.Offset), SEEK_SET);
	if (source.Read(stored.data(), int(stored.size())) != int(stored.size())) {
		return(records);
	}
	Bytes stream;
	if (frames.Codec == std::uint32_t(Codec::DEFLATE)) {
		Inflate_All(stored, stream, ~0ull);
	} else {
		stream = stored;
	}

	std::size_t offset = 0;
	std::int64_t previous = -1;
	for (;;) {
		std::uint64_t length = 0;
		if (Read_Varint(stream, offset, length) != WireError::NONE || length == 0 || length > stream.size() - offset) {
			return(records);
		}
		Bytes body(stream.begin() + offset, stream.begin() + offset + std::size_t(length));
		Frame record;
		if (!Decode(body, record)) {
			return(records);
		}
		previous += 1 + std::int64_t(record.Skipped);
		records.emplace_back(std::uint32_t(previous), std::move(body));
		offset += std::size_t(length);
	}
}


// Checks a condition the rest of one file's checks depend on, and stops checking that file
// when it fails, so that the other files are still checked.
#define CHECK_OR_STOP(condition) \
	do { \
		bool const holds = bool(condition); \
		CHECK_MESSAGE(holds, #condition); \
		if (!holds) { \
			return; \
		} \
	} while (false)


void Check_File(Json const & entry)
{
	std::string const name = entry["file"].Text;
	CAPTURE(name);
	ByteVector const file = Load(Vectors_Dir() / name);
	Judgement const verdict = Judge(file);

	if (entry.Has("s0")) {
		CHECK_OR_STOP(verdict.S0.has_value());
		CHECK(Hex(*verdict.S0) == entry["s0"].Text);
	}

	if (entry.Has("records")) {
		CHECK(std::int64_t(verdict.Records.size()) == entry["records"].Number);
	}
	if (entry.Find("last_recorded_frame") != nullptr) {
		if (entry.Has("last_recorded_frame")) {
			CHECK_OR_STOP(!verdict.Records.empty());
			CHECK(std::int64_t(verdict.Records.back().first) == entry["last_recorded_frame"].Number);
		} else {
			CHECK(verdict.Records.empty());
		}
	}

	if (entry.Has("seals")) {
		std::string const expected = Seal_List(entry["seals"]);
		CHECK_MESSAGE(Seal_List(verdict.Chain) == expected, "the chain C++ computes");
		CHECK_OR_STOP(verdict.HasSummary);
		CHECK_MESSAGE(Seal_List(verdict.Summary.Seals) == expected, "the seals the file's summary carries");
	}

	if (entry.Has("frame_seals")) {
		for (auto const & member : entry["frame_seals"].Members) {
			std::uint32_t const frame = std::uint32_t(std::stoul(member.first));
			CAPTURE(frame);
			std::vector<std::uint32_t> expected;
			for (Json const & through : member.second.Items) {
				expected.push_back(std::uint32_t(through.Number));
			}
			Frame const * record = Record_At(verdict, frame);
			CHECK_OR_STOP(record != nullptr);
			CHECK(Throughs(record->Seals) == expected);
			auto const closed = verdict.Closed.find(frame);
			CHECK_OR_STOP(closed != verdict.Closed.end());
			CHECK(Throughs(closed->second) == expected);
			CHECK(Seal_List(closed->second) == Seal_List(record->Seals));
		}
	}

	// Canonical bytes come from the stored records, since a file can be malformed for a record
	// whose canonical bytes are still defined.
	if (entry.Has("canonical")) {
		auto const stored = Stored_Records(file);
		for (Json const & canonical : entry["canonical"].Items) {
			std::uint32_t const frame = std::uint32_t(canonical["frame"].Number);
			CAPTURE(frame);
			auto const found = std::find_if(stored.begin(), stored.end(), [&](auto const & item) { return(item.first == frame); });
			CHECK_OR_STOP(found != stored.end());
			Frame record;
			CHECK_OR_STOP(Decode(found->second, record));
			Bytes bytes;
			Canonical_Frame_Bytes(frame, record, bytes);
			CHECK(Hex(bytes) == canonical["bytes"].Text);
		}
	}

	if (entry.Has("ckpt0_body")) {
		CHECK_OR_STOP(verdict.CheckpointBodies.size() >= 2);
		CHECK(Hex(verdict.CheckpointBodies[0]) == entry["ckpt0_body"].Text);
		CHECK(Hex(verdict.CheckpointBodies[1]) == entry["ckpt1_body"].Text);
	}

	if (entry.Has("record_body")) {
		auto const stored = Stored_Records(file);
		CHECK_OR_STOP(!stored.empty());
		CHECK_MESSAGE(Hex(stored.front().second) == entry["record_body"].Text, "the first record as stored");
	}

	if (entry.Has("stat_body")) {
		CHECK(Hex(verdict.StatBody) == entry["stat_body"].Text);
		Statistics statistics;
		CHECK_OR_STOP(Decode(verdict.StatBody, statistics));
		CHECK_MESSAGE(Hex(Encoded(statistics)) == entry["stat_body"].Text, "the statistics encoded again");
	}

	if (entry.Has("event_payload")) {
		Event const * found = nullptr;
		for (auto const & item : verdict.Records) {
			for (Event const & event : item.second.Events) {
				if (event.SealedPayload && found == nullptr) {
					found = &event;
				}
			}
		}
		CHECK_OR_STOP(found != nullptr);
		CHECK(Hex(found->Payload) == entry["event_payload"].Text);
		if (entry.Has("event_sealed_payload")) {
			CHECK(Hex(*found->SealedPayload) == entry["event_sealed_payload"].Text);
		}
	}

	if (entry.Has("player_name_hex")) {
		CHECK_OR_STOP(verdict.Start.Players.size() >= 2);
		CHECK(Hex_Text(verdict.Start.Players[1].Name) == entry["player_name_hex"].Text);
	}

	if (entry.Has("game_micros")) {
		CHECK_OR_STOP(verdict.HasSummary);
		CHECK_OR_STOP(verdict.Summary.GameMicros.has_value());
		CHECK(std::int64_t(*verdict.Summary.GameMicros) == entry["game_micros"].Number);
	}
}

}


TEST_CASE("the Python vector files' S0, seals, and bytes match the manifest")
{
	for (Json const & entry : Manifest()["files"].Items) {
		Check_File(entry);
	}
}
