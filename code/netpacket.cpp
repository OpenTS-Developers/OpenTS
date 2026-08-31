/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#include "netpacket.h"

#include "netreader.h"

#include <array>
#include <cstring>
#include <limits>
#include <type_traits>
#include <utility>


namespace NetPacket
{
	namespace {

		using EventDataType = decltype(std::declval<EventClass>().Data);
		using FrameInfoType = decltype(std::declval<EventClass>().Data.FrameInfo);
		using MegaMissionType = decltype(std::declval<EventClass>().Data.MegaMission);
		using VariableDataType = decltype(std::declval<EventClass>().Data.Variable);
		using VariableSizeType = decltype(std::declval<VariableDataType>().Size);
		using EventTypeField = decltype(std::declval<EventClass>().Type);
		using EventFrameField = decltype(std::declval<EventClass>().Frame);
		using EventExecutedField = decltype(std::declval<EventClass>().IsExecuted);
		using EventSenderField = decltype(std::declval<EventClass>().ID);

		constexpr std::size_t EVENT_FRAME_OFFSET = offsetof(EventClass, Frame);
		constexpr std::size_t EVENT_SENDER_OFFSET = offsetof(EventClass, ID);
		constexpr std::size_t EVENT_DATA_OFFSET = offsetof(EventClass, Data);
		constexpr std::size_t EVENT_DATA_SIZE = sizeof(EventDataType);
		constexpr std::size_t FRAMEINFO_DELAY_OFFSET = offsetof(FrameInfoType, Delay);
		constexpr std::size_t VARIABLE_SIZE_OFFSET = offsetof(VariableDataType, Size);
		constexpr std::size_t MEGAMISSION_WHOM_OFFSET = offsetof(MegaMissionType, Whom);
		constexpr std::size_t MEGAMISSION_WHOM_SIZE = sizeof(std::declval<MegaMissionType>().Whom);

		static_assert(std::is_standard_layout_v<EventClass>);
		static_assert(std::is_trivially_copyable_v<EventClass>);
		static_assert(EventClass::LAST_EVENT <= (std::numeric_limits<EventTypeField>::max)());


		// Pending events keep packet decoding transactional until every byte is validated.
		struct PendingEvent
		{
			std::uint8_t Type = EventClass::EMPTY;
			int Frame = 0;
			int Sender = 0;
			std::array<std::byte, EVENT_DATA_SIZE> Data{};
			std::vector<std::byte> AddPlayerData;
		};


		struct PacketEnvelope
		{
			std::uint8_t Type = EventClass::EMPTY;
			int Frame = 0;
			int Sender = 0;
			std::array<std::byte, sizeof(FrameInfoType)> FrameInfo{};
		};


		/// <summary>Builds a packet-decode failure at a stable byte offset.</summary>
		DecodeResult Failed(DecodeError code, std::size_t offset, std::uint8_t event_type = NO_EVENT_TYPE)
		{
			DecodeResult result;
			result.Failure.Code = code;
			result.Failure.Offset = offset;
			result.Failure.EventType = event_type;
			return(result);
		}


		/// <summary>Checks an event type before table lookup.</summary>
		bool Is_Known_Event(std::uint8_t type)
		{
			return(type < EventClass::LAST_EVENT);
		}


		/// <summary>Identifies packet-level synchronization events.</summary>
		bool Is_Envelope(std::uint8_t type)
		{
			return(type == EventClass::FRAMEINFO || type == EventClass::FRAMESYNC);
		}


		/// <summary>Reads a complete frame envelope from bounded packet bytes.</summary>
		bool Read_Envelope(Reader & reader, PacketEnvelope & envelope, DecodeFailure & failure)
		{
			std::size_t const offset = reader.Offset();
			auto type = reader.Read_Value<EventTypeField>();
			auto frame = reader.Read_Value<EventFrameField>();
			auto executed = reader.Take(sizeof(EventExecutedField));
			auto sender = reader.Read_Value<EventSenderField>();
			auto frame_info = reader.Take(sizeof(FrameInfoType));

			if (!type || !frame || !executed || !sender || !frame_info) {
				failure.Code = DecodeError::TRUNCATED_ENVELOPE;
				failure.Offset = offset;
				failure.EventType = type.value_or(NO_EVENT_TYPE);
				return(false);
			}

			envelope.Type = *type;
			envelope.Frame = *frame;
			envelope.Sender = *sender;
			std::memcpy(envelope.FrameInfo.data(), frame_info->data(), envelope.FrameInfo.size());
			return(true);
		}


		/// <summary>Validates the sender-frame arithmetic retained in an envelope.</summary>
		bool Validate_Envelope_Frame(int frame, std::span<std::byte const> frame_info, std::uint8_t type, DecodeFailure & failure)
		{
			std::uint8_t delay = 0;
			std::memcpy(&delay, frame_info.data() + FRAMEINFO_DELAY_OFFSET, sizeof(delay));
			if (Compute_Reported_Frame(frame, delay)) {
				return(true);
			}

			failure.Code = DecodeError::INVALID_FRAME_ARITHMETIC;
			failure.Offset = frame < 0 ? EVENT_FRAME_OFFSET : EVENT_DATA_OFFSET + FRAMEINFO_DELAY_OFFSET;
			failure.EventType = type;
			return(false);
		}


		/// <summary>Converts an admitted envelope into a pending event.</summary>
		PendingEvent Pending_From_Envelope(PacketEnvelope const & envelope)
		{
			PendingEvent event;
			event.Type = envelope.Type;
			event.Frame = envelope.Frame;
			event.Sender = envelope.Sender;
			std::memcpy(event.Data.data(), envelope.FrameInfo.data(), envelope.FrameInfo.size());
			return(event);
		}


		/// <summary>Fetches a validated event payload length.</summary>
		bool Event_Data_Length(std::uint8_t type, std::size_t offset, std::size_t & length, DecodeFailure & failure)
		{
			length = EventClass::EventLength[type];
			if (length <= EVENT_DATA_SIZE) {
				return(true);
			}

			failure.Code = DecodeError::INVALID_EVENT_LENGTH;
			failure.Offset = offset;
			failure.EventType = type;
			return(false);
		}


		/// <summary>Places compact wire data into its EventClass field.</summary>
		void Copy_Event_Data(PendingEvent & event, std::uint8_t type, std::span<std::byte const> data)
		{
			std::size_t offset = 0;
			if (type == EventClass::RESPONSE_TIME) {
				offset = FRAMEINFO_DELAY_OFFSET;
			}

			std::memcpy(event.Data.data() + offset, data.data(), data.size());
		}


		/// <summary>Reads and owns a variable-length ADDPLAYER payload.</summary>
		bool Read_Add_Player(Reader & reader, PendingEvent & event, std::size_t event_offset, DecodeFailure & failure)
		{
			auto size = reader.Read_Value<VariableSizeType>();
			if (!size) {
				failure.Code = DecodeError::TRUNCATED_ADDPLAYER;
				failure.Offset = event_offset;
				failure.EventType = EventClass::ADDPLAYER;
				return(false);
			}

			auto data = reader.Take(*size);
			if (!data) {
				failure.Code = DecodeError::TRUNCATED_ADDPLAYER;
				failure.Offset = event_offset;
				failure.EventType = EventClass::ADDPLAYER;
				return(false);
			}

			std::memcpy(event.Data.data() + VARIABLE_SIZE_OFFSET, &*size, sizeof(*size));
			event.AddPlayerData.assign(data->begin(), data->end());
			return(true);
		}


		/// <summary>Expands a bounded compressed MEGAMISSION run into pending events.</summary>
		bool Read_Compressed_Mega_Mission(Reader & reader, int frame, int sender, std::size_t event_offset,
			std::vector<PendingEvent> & events, DecodeFailure & failure)
		{
			auto count = reader.Read_Value<std::uint8_t>();
			if (!count) {
				failure.Code = DecodeError::TRUNCATED_MEGAMISSION;
				failure.Offset = event_offset;
				failure.EventType = EventClass::MEGAMISSION;
				return(false);
			}
			if (*count == 0) {
				failure.Code = DecodeError::ZERO_MEGAMISSION_COUNT;
				failure.Offset = event_offset;
				failure.EventType = EventClass::MEGAMISSION;
				return(false);
			}

			std::size_t const data_length = EventClass::EventLength[EventClass::MEGAMISSION];
			auto data = reader.Take(data_length);
			if (!data) {
				failure.Code = DecodeError::TRUNCATED_MEGAMISSION;
				failure.Offset = event_offset;
				failure.EventType = EventClass::MEGAMISSION;
				return(false);
			}

			PendingEvent first;
			first.Type = EventClass::MEGAMISSION;
			first.Frame = frame;
			first.Sender = sender;
			std::memcpy(first.Data.data(), data->data(), data->size());
			events.push_back(first);

			for (std::uint8_t index = 1; index < *count; index++) {
				auto whom = reader.Take(MEGAMISSION_WHOM_SIZE);
				if (!whom) {
					failure.Code = DecodeError::TRUNCATED_MEGAMISSION;
					failure.Offset = event_offset;
					failure.EventType = EventClass::MEGAMISSION;
					return(false);
				}

				PendingEvent repeated = first;
				std::memcpy(repeated.Data.data() + MEGAMISSION_WHOM_OFFSET, whom->data(), whom->size());
				events.push_back(std::move(repeated));
			}

			return(true);
		}


		/// <summary>Materializes a completely validated batch of pending events.</summary>
		DecodeResult Materialize(std::vector<PendingEvent> pending)
		{
			DecodeResult result;
			result.Events.reserve(pending.size());

			for (PendingEvent & source : pending) {
				EventClass event;
				std::memset(&event, 0, sizeof(event));
				event.Type = source.Type;
				event.Frame = source.Frame;
				event.IsExecuted = false;
				event.ID = source.Sender;
				std::memcpy(&event.Data, source.Data.data(), source.Data.size());

				result.Events.emplace_back(event, std::move(source.AddPlayerData));
			}
			if (!result.Events.empty()) {
				result.Envelope = result.Events.front().Event;
				result.HasEnvelope = true;
			}

			return(result);
		}


		/// <summary>Preserves a validated FRAMESYNC envelope without scheduling it as an event.</summary>
		DecodeResult Materialize_Frame_Sync(PacketEnvelope const & envelope)
		{
			DecodeResult result = Materialize({Pending_From_Envelope(envelope)});
			result.Events.clear();
			return(result);
		}


		/// <summary>Decodes a compressed event packet transactionally.</summary>
		DecodeResult Decode_Compressed(std::span<std::byte const> packet, int expected_sender)
		{
			Reader reader(packet);
			std::uint8_t const first_type = std::to_integer<std::uint8_t>(packet.front());

			if (!Is_Known_Event(first_type)) {
				return(Failed(DecodeError::INVALID_EVENT_TYPE, 0, first_type));
			}
			if (!Is_Envelope(first_type)) {
				return(Failed(DecodeError::INVALID_PREFIX, 0, first_type));
			}

			PacketEnvelope envelope;
			DecodeFailure failure;
			if (!Read_Envelope(reader, envelope, failure)) {
				DecodeResult result;
				result.Failure = failure;
				return(result);
			}
			if (envelope.Sender != expected_sender) {
				return(Failed(DecodeError::SENDER_MISMATCH, EVENT_SENDER_OFFSET, envelope.Type));
			}
			if (envelope.Type == EventClass::FRAMESYNC) {
				if (!reader.Empty()) {
					return(Failed(DecodeError::FRAMESYNC_NOT_ALONE, reader.Offset(), envelope.Type));
				}
				if (!Validate_Envelope_Frame(envelope.Frame, envelope.FrameInfo, envelope.Type, failure)) {
					DecodeResult result;
					result.Failure = failure;
					return(result);
				}
				return(Materialize_Frame_Sync(envelope));
			}
			if (!Validate_Envelope_Frame(envelope.Frame, envelope.FrameInfo, envelope.Type, failure)) {
				DecodeResult result;
				result.Failure = failure;
				return(result);
			}

			// Compact children inherit the identity from the already validated envelope.
			std::vector<PendingEvent> events;
			events.push_back(Pending_From_Envelope(envelope));

			while (!reader.Empty()) {
				std::size_t const event_offset = reader.Offset();
				auto type_value = reader.Read_Value<std::uint8_t>();
				if (!type_value) {
					return(Failed(DecodeError::TRUNCATED_EVENT, event_offset));
				}

				std::uint8_t const type = *type_value;
				if (!Is_Known_Event(type)) {
					return(Failed(DecodeError::INVALID_EVENT_TYPE, event_offset, type));
				}
				if (Is_Envelope(type)) {
					return(Failed(DecodeError::NESTED_ENVELOPE, event_offset, type));
				}

				if (type == EventClass::MEGAMISSION) {
					if (!Read_Compressed_Mega_Mission(reader, envelope.Frame, envelope.Sender, event_offset, events, failure)) {
						DecodeResult result;
						result.Failure = failure;
						return(result);
					}
					continue;
				}

				PendingEvent event;
				event.Type = type;
				event.Frame = envelope.Frame;
				event.Sender = envelope.Sender;

				if (type == EventClass::ADDPLAYER) {
					if (EventClass::EventLength[type] != sizeof(VariableSizeType)) {
						return(Failed(DecodeError::INVALID_EVENT_LENGTH, event_offset, type));
					}
					if (!Read_Add_Player(reader, event, event_offset, failure)) {
						DecodeResult result;
						result.Failure = failure;
						return(result);
					}
					events.push_back(std::move(event));
					continue;
				}

				std::size_t data_length = 0;
				if (!Event_Data_Length(type, event_offset, data_length, failure)) {
					DecodeResult result;
					result.Failure = failure;
					return(result);
				}
				auto data = reader.Take(data_length);
				if (!data) {
					return(Failed(DecodeError::TRUNCATED_EVENT, event_offset, type));
				}

				Copy_Event_Data(event, type, *data);
				events.push_back(std::move(event));
			}

			return(Materialize(std::move(events)));
		}


		/// <summary>Reads one fixed-size EventClass record into a pending event.</summary>
		bool Read_Full_Event(std::span<std::byte const> bytes, PendingEvent & event, std::size_t event_offset, int expected_sender, DecodeFailure & failure)
		{
			Reader reader(bytes);
			auto type = reader.Read_Value<EventTypeField>();
			auto frame = reader.Read_Value<EventFrameField>();
			auto executed = reader.Take(sizeof(EventExecutedField));
			auto sender = reader.Read_Value<EventSenderField>();
			auto data = reader.Take(EVENT_DATA_SIZE);

			if (!type || !frame || !executed || !sender || !data) {
				failure.Code = DecodeError::TRUNCATED_EVENT;
				failure.Offset = event_offset;
				failure.EventType = type.value_or(NO_EVENT_TYPE);
				return(false);
			}
			if (!Is_Known_Event(*type)) {
				failure.Code = DecodeError::INVALID_EVENT_TYPE;
				failure.Offset = event_offset;
				failure.EventType = *type;
				return(false);
			}
			if (*sender != expected_sender) {
				failure.Code = DecodeError::SENDER_MISMATCH;
				failure.Offset = event_offset + EVENT_SENDER_OFFSET;
				failure.EventType = *type;
				return(false);
			}

			event.Type = *type;
			event.Frame = *frame;
			event.Sender = *sender;

			std::size_t data_length = 0;
			if (!Event_Data_Length(*type, event_offset, data_length, failure)) {
				return(false);
			}

			if (*type == EventClass::FRAMEINFO) {
				std::memcpy(event.Data.data(), data->data(), sizeof(FrameInfoType));
			} else if (*type == EventClass::ADDPLAYER) {
				std::memcpy(event.Data.data() + VARIABLE_SIZE_OFFSET, data->data() + VARIABLE_SIZE_OFFSET, sizeof(VariableSizeType));
			} else if (*type == EventClass::RESPONSE_TIME) {
				std::memcpy(event.Data.data() + FRAMEINFO_DELAY_OFFSET, data->data() + FRAMEINFO_DELAY_OFFSET, data_length);
			} else {
				std::memcpy(event.Data.data(), data->data(), data_length);
			}

			return(true);
		}


		/// <summary>Reads the variable payload size retained in an ADDPLAYER event.</summary>
		VariableSizeType Add_Player_Size(PendingEvent const & event)
		{
			VariableSizeType size = 0;
			std::memcpy(&size, event.Data.data() + VARIABLE_SIZE_OFFSET, sizeof(size));
			return(size);
		}


		/// <summary>Decodes an uncompressed event packet transactionally.</summary>
		DecodeResult Decode_Uncompressed(std::span<std::byte const> packet, int expected_sender)
		{
			std::uint8_t const first_type = std::to_integer<std::uint8_t>(packet.front());
			if (!Is_Known_Event(first_type)) {
				return(Failed(DecodeError::INVALID_EVENT_TYPE, 0, first_type));
			}
			if (!Is_Envelope(first_type)) {
				return(Failed(DecodeError::INVALID_PREFIX, 0, first_type));
			}

			if (first_type == EventClass::FRAMESYNC) {
				Reader reader(packet);
				PacketEnvelope envelope;
				DecodeFailure failure;
				if (!Read_Envelope(reader, envelope, failure)) {
					DecodeResult result;
					result.Failure = failure;
					return(result);
				}
				if (envelope.Sender != expected_sender) {
					return(Failed(DecodeError::SENDER_MISMATCH, EVENT_SENDER_OFFSET, envelope.Type));
				}
				if (!reader.Empty()) {
					return(Failed(DecodeError::FRAMESYNC_NOT_ALONE, reader.Offset(), envelope.Type));
				}
				if (!Validate_Envelope_Frame(envelope.Frame, envelope.FrameInfo, envelope.Type, failure)) {
					DecodeResult result;
					result.Failure = failure;
					return(result);
				}
				return(Materialize_Frame_Sync(envelope));
			}
			if (packet.size() < sizeof(EventClass)) {
				return(Failed(DecodeError::TRUNCATED_ENVELOPE, 0, first_type));
			}

			Reader reader(packet);
			std::vector<PendingEvent> events;
			DecodeFailure failure;

			while (!reader.Empty()) {
				std::size_t const event_offset = reader.Offset();
				if (reader.Remaining() < sizeof(EventClass)) {
					return(Failed(DecodeError::TRAILING_BYTES, event_offset));
				}

				auto bytes = reader.Take(sizeof(EventClass));
				PendingEvent event;
				if (!Read_Full_Event(*bytes, event, event_offset, expected_sender, failure)) {
					DecodeResult result;
					result.Failure = failure;
					return(result);
				}

				if (events.empty()) {
					if (event.Type != EventClass::FRAMEINFO) {
						return(Failed(DecodeError::INVALID_PREFIX, event_offset, event.Type));
					}
					if (!Validate_Envelope_Frame(event.Frame, event.Data, event.Type, failure)) {
						DecodeResult result;
						result.Failure = failure;
						return(result);
					}
				} else if (Is_Envelope(event.Type)) {
					return(Failed(DecodeError::NESTED_ENVELOPE, event_offset, event.Type));
				}

				if (event.Type == EventClass::ADDPLAYER) {
					VariableSizeType const size = Add_Player_Size(event);
					auto data = reader.Take(size);
					if (!data) {
						return(Failed(DecodeError::TRUNCATED_ADDPLAYER, event_offset, event.Type));
					}
					event.AddPlayerData.assign(data->begin(), data->end());
				}

				events.push_back(std::move(event));
			}

			return(Materialize(std::move(events)));
		}

	}	// namespace


	/// <summary>Constructs an empty decoded event.</summary>
	DecodedEvent::DecodedEvent(void) noexcept
	{
		std::memset(&Event, 0, sizeof(Event));
	}


	/// <summary>Owns one decoded event and its optional variable payload.</summary>
	DecodedEvent::DecodedEvent(EventClass const & event, std::vector<std::byte> add_player_data) noexcept
		: Event(event), AddPlayerData(std::move(add_player_data))
	{
		Bind_AddPlayer_Data();
	}


	/// <summary>Copies a decoded event and repairs its owned payload pointer.</summary>
	DecodedEvent::DecodedEvent(DecodedEvent const & other)
		: Event(other.Event), AddPlayerData(other.AddPlayerData)
	{
		Bind_AddPlayer_Data();
	}


	/// <summary>Moves a decoded event and repairs both payload pointers.</summary>
	DecodedEvent::DecodedEvent(DecodedEvent && other) noexcept
		: Event(other.Event), AddPlayerData(std::move(other.AddPlayerData))
	{
		Bind_AddPlayer_Data();
		other.Bind_AddPlayer_Data();
	}


	/// <summary>Copies a decoded event and repairs its owned payload pointer.</summary>
	DecodedEvent & DecodedEvent::operator=(DecodedEvent const & other)
	{
		if (this != &other) {
			Event = other.Event;
			AddPlayerData = other.AddPlayerData;
			Bind_AddPlayer_Data();
		}
		return(*this);
	}


	/// <summary>Moves a decoded event and repairs both payload pointers.</summary>
	DecodedEvent & DecodedEvent::operator=(DecodedEvent && other) noexcept
	{
		if (this != &other) {
			Event = other.Event;
			AddPlayerData = std::move(other.AddPlayerData);
			Bind_AddPlayer_Data();
			other.Bind_AddPlayer_Data();
		}
		return(*this);
	}


	/// <summary>Binds an ADDPLAYER event to its owned variable payload.</summary>
	void DecodedEvent::Bind_AddPlayer_Data(void) noexcept
	{
		if (Event.Type != EventClass::ADDPLAYER) {
			return;
		}

		Event.Data.Variable.Size = static_cast<unsigned int>(AddPlayerData.size());
		Event.Data.Variable.Pointer = AddPlayerData.empty() ? nullptr : AddPlayerData.data();
	}


	/// <summary>Checks whether packet decoding completed without error.</summary>
	bool DecodeResult::Succeeded(void) const noexcept
	{
		return(Failure.Code == DecodeError::NONE);
	}


	/// <summary>Computes the sender frame without signed underflow.</summary>
	std::optional<std::int64_t> Compute_Reported_Frame(int event_frame, std::uint8_t delay) noexcept
	{
		std::int64_t const frame = event_frame;
		std::int64_t const frame_delay = delay;
		if (frame < 0 || frame_delay > frame) {
			return(std::nullopt);
		}

		return(frame - frame_delay);
	}


	/// <summary>Bounds a reported sender frame against the receiver's current frame.</summary>
	std::optional<std::int64_t> Compute_Reported_Frame(int event_frame, std::uint8_t delay, int receiver_frame, std::uint32_t maximum_lead) noexcept
	{
		std::optional<std::int64_t> const reported = Compute_Reported_Frame(event_frame, delay);
		std::int64_t const maximum = static_cast<std::int64_t>(receiver_frame) + maximum_lead;
		return(reported && *reported <= maximum ? reported : std::nullopt);
	}


	/// <summary>Decodes a complete event packet using its negotiated encoding.</summary>
	DecodeResult Decode_Event_Packet(std::span<std::byte const> packet, Encoding encoding, int expected_sender)
	{
		if (packet.empty()) {
			return(Failed(DecodeError::EMPTY_PACKET, 0));
		}

		switch (encoding) {
			case Encoding::UNCOMPRESSED:
				return(Decode_Uncompressed(packet, expected_sender));

			case Encoding::COMPRESSED:
				return(Decode_Compressed(packet, expected_sender));
		}

		return(Failed(DecodeError::INVALID_PREFIX, 0));
	}


	/// <summary>Returns a stable packet-decode error name.</summary>
	char const * Error_Name(DecodeError error) noexcept
	{
		switch (error) {
			case DecodeError::NONE: return("none");
			case DecodeError::EMPTY_PACKET: return("empty packet");
			case DecodeError::INVALID_EVENT_TYPE: return("invalid event type");
			case DecodeError::INVALID_PREFIX: return("invalid packet prefix");
			case DecodeError::TRUNCATED_ENVELOPE: return("truncated packet envelope");
			case DecodeError::FRAMESYNC_NOT_ALONE: return("framesync is not alone");
			case DecodeError::NESTED_ENVELOPE: return("nested packet envelope");
			case DecodeError::SENDER_MISMATCH: return("sender identity mismatch");
			case DecodeError::INVALID_FRAME_ARITHMETIC: return("invalid frame arithmetic");
			case DecodeError::INVALID_EVENT_LENGTH: return("invalid event length");
			case DecodeError::TRUNCATED_EVENT: return("truncated event");
			case DecodeError::ZERO_MEGAMISSION_COUNT: return("zero megamission count");
			case DecodeError::TRUNCATED_MEGAMISSION: return("truncated megamission");
			case DecodeError::TRUNCATED_ADDPLAYER: return("truncated add-player data");
			case DecodeError::TRAILING_BYTES: return("trailing packet bytes");
			case DecodeError::INVALID_CONNECTION: return("invalid connection");
			case DecodeError::COUNT: break;
		}

		return("unknown packet error");
	}
}
