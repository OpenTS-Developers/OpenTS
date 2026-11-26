/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#include "crc.h"
#include "sha256.h"

#include "replaytest.h"

#include "doctest/doctest.h"

#include <string>


static std::string SHA256_Hex(std::string_view text)
{
	return(Hex(SHA256Class::Digest(text.data(), text.size())));
}


TEST_CASE("SHA-256 matches the FIPS 180-2 test vectors")
{
	CHECK(SHA256_Hex("") == "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855");
	CHECK(SHA256_Hex("abc") == "ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad");
	CHECK(SHA256_Hex("abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq")
		== "248d6a61d20638b8e5c026930c3e6039a33ce45964ff2167f6ecedd419db06c1");
	CHECK(SHA256_Hex("abcdefghbcdefghicdefghijdefghijkefghijklfghijklmghijklmnhijklmnoijklmnopjklmnopqklmnopqrlmnopqrsmnopqrstnopqrstu")
		== "cf5b16a778af8380036ce59e7b0492370b249b11e8f07a51afac45037afee9d1");

	SHA256Class engine;
	std::string const thousand(1000, 'a');
	for (int i = 0; i < 1000; i++) {
		engine.Hash(thousand.data(), thousand.size());
	}
	CHECK_MESSAGE(Hex(engine.Result()) == "cdc76e5c9914fb9281a1c7e284d73e67f1809a48a497200e046d39ccc7112cd0",
		"one million 'a' hashed in pieces of 1000");
}


TEST_CASE("SHA-256 gives one digest however the input is split")
{
	ByteVector data(200);
	for (std::size_t i = 0; i < data.size(); i++) {
		data[i] = std::uint8_t(i * 7 + 3);
	}
	auto const whole = SHA256Class::Digest(data);

	for (std::size_t cut = 0; cut <= data.size(); cut++) {
		SHA256Class engine;
		engine.Hash(data.data(), cut);
		engine.Hash(data.data() + cut, data.size() - cut);
		CHECK_MESSAGE(engine.Result() == whole, "split at ", cut);
	}

	SHA256Class bytewise;
	for (std::uint8_t byte : data) {
		bytewise.Hash(&byte, 1);
	}
	CHECK(bytewise.Result() == whole);
}


TEST_CASE("SHA-256 can continue after a result and start over after a reset")
{
	SHA256Class engine;
	engine.Hash("ab", 2);
	CHECK(Hex(engine.Result()) == SHA256_Hex("ab"));
	engine.Hash("c", 1);
	CHECK(Hex(engine.Result()) == SHA256_Hex("abc"));
	engine.Reset();
	CHECK(Hex(engine.Result()) == SHA256_Hex(""));
}


TEST_CASE("CRC::Memory is the CRC-32 that zlib computes")
{
	unsigned char const check[] = "123456789";
	CHECK(CRC::Memory(check, 9) == 0xCBF43926u);
	CHECK(CRC::Memory(check + 4, 5, CRC::Memory(check, 4)) == 0xCBF43926u);
	CHECK(CRC::Memory(check, 0) == 0u);
}
