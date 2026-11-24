// Drives the pointer swizzler a saved game is loaded through: a request whose identity nobody
// announced fails the resolve instead of ending the process, and the tables are empty after
// either result. It reads no game data.

#include "swizzle.h"

#include "doctest/doctest.h"

namespace {

int First = 1;
int Second = 2;

}	// namespace


TEST_CASE("every announced identity is written into the pointers that asked for it")
{
	Swizzler.Discard();

	void * one = &Second;
	void * two = &Second;
	Swizzler.Swizzle(&one, 7);
	Swizzler.Swizzle(&two, 9);
	CHECK(one == nullptr);
	CHECK(two == nullptr);

	Swizzler.Here_I_Am(9, &Second);
	Swizzler.Here_I_Am(7, &First);

	CHECK(Swizzler.Resolve());
	CHECK(one == &First);
	CHECK(two == &Second);
}


TEST_CASE("an identity nobody announced fails the resolve and leaves its pointer null")
{
	Swizzler.Discard();

	void * matched = nullptr;
	void * orphan = &First;
	Swizzler.Swizzle(&matched, 3);
	Swizzler.Swizzle(&orphan, 4, "OwnerClass", 3, "SlotClass", __FILE__, __LINE__);
	Swizzler.Here_I_Am(3, &Second);

	CHECK_FALSE(Swizzler.Resolve());
	CHECK(matched == &Second);
	CHECK(orphan == nullptr);

	// Nothing of the failed load is left to resolve into the next one.
	Swizzler.Here_I_Am(4, &First);
	CHECK(Swizzler.Resolve());
	CHECK(orphan == nullptr);
}


TEST_CASE("a null identity needs no announcement")
{
	Swizzler.Discard();

	void * pointer = &First;
	Swizzler.Swizzle(&pointer, 0);
	CHECK(pointer == nullptr);
	CHECK(Swizzler.Resolve());
}
