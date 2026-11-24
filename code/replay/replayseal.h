/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#pragma once

#include "replay/replaymessages.h"
#include "sha256.h"

#include <cstddef>
#include <cstdint>
#include <map>
#include <optional>
#include <string_view>
#include <vector>

// The seal chain of the replay format, seal version 1.

namespace Replay
{
	using Digest = SHA256Class::DigestType;

	// S0, from the INFO values and the uncompressed LNCH and INIT bodies.
	Digest Seal_Start(std::uint32_t seal_version, std::string_view engine, std::uint32_t data_version,
		ByteSpan launch, ByteSpan init);

	// Appends the record's canonical bytes to out; false, with nothing appended, when it has none.
	bool Canonical_Frame_Bytes(std::uint32_t frame, Message::Frame const & record, Bytes & out);

	// Takes records in ascending frame order.
	class SealChain
	{
		public:
			explicit SealChain(Digest const & start);

			// Adds the record for frame; closed receives the interval seals that belong on that record.
			void Add(std::uint32_t frame, Message::Frame const & record, std::vector<Message::Seal> & closed);

			// Appends the seals left through last_frame, the final seal last, and ends the chain.
			void Finish(std::uint32_t last_frame, std::vector<Message::Seal> & seals);

			// The number of unsealed intervals that end before frame.
			std::uint64_t Intervals_Before(std::uint32_t frame) const;

			// The last frame of the interval the next interval seal closes.
			std::uint64_t Interval_End(void) const { return(IntervalEnd); }

		private:
			void Close_Interval(std::vector<Message::Seal> & seals);

			SHA256Class Hash;
			std::uint64_t IntervalEnd;   // last frame of the open interval
	};

	// Recomputes the chain from a file's records and compares it with the seals in them.
	class SealVerifier
	{
		public:
			explicit SealVerifier(Digest const & start);

			void Add(std::uint32_t frame, Message::Frame const & record);

			// Compares the summary's seals with the whole chain.
			void Finish(Message::RecordingSummary const & summary);

			bool Good(void) const { return(!Mismatch.has_value() && !Misplaced.has_value()); }
			std::size_t Compared(void) const { return(Matched); }

			// The through of the first seal that differs from the chain or it could not produce.
			std::optional<std::uint32_t> First_Mismatch(void) const { return(Mismatch); }

			// The lowest through of a seal missing or misplaced, which makes the file malformed.
			std::optional<std::uint32_t> First_Misplaced(void) const { return(Misplaced); }

		private:
			void Compare(Message::Seal const & seal);

			SealChain Chain;
			std::map<std::uint32_t, Bytes> Computed;
			std::optional<std::uint32_t> LastFrame;
			std::optional<std::uint32_t> Mismatch;
			std::optional<std::uint32_t> Misplaced;
			std::size_t Matched;
			bool Stopped;
	};
}
