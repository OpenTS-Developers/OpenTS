/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#include "netadmit.h"

#include <cstring>


namespace {

constexpr std::size_t CRC_SIZE = sizeof(std::uint32_t);
constexpr std::size_t COMMON_HEADER_SIZE = NET_PRIVATE_HEADER_SIZE;


/// <summary>Folds one native-endian word into the legacy network CRC.</summary>
void Add_CRC_Value(std::uint32_t & crc, std::uint32_t value) noexcept
{
	std::uint32_t const high_bit = crc >> 31;
	crc = (crc << 1) + value + high_bit;
}

} // namespace


/// <summary>Calculates the legacy CRC over a datagram payload.</summary>
std::uint32_t Calculate_Network_Datagram_CRC(std::span<std::byte const> payload) noexcept
{
	std::uint32_t crc = 0;
	std::size_t position = 0;
	while (payload.size() - position >= sizeof(std::uint32_t)) {
		std::uint32_t value = 0;
		std::memcpy(&value, payload.data() + position, sizeof(value));
		Add_CRC_Value(crc, value);
		position += sizeof(value);
	}

	if (position < payload.size()) {
		std::uint32_t value = 0;
		std::memcpy(&value, payload.data() + position, payload.size() - position);
		Add_CRC_Value(crc, value);
	}
	return(crc);
}


/// <summary>Validates a datagram's capacity and CRC.</summary>
NetDatagramAdmission Admit_Network_Datagram(std::span<std::byte const> datagram, std::size_t payload_capacity) noexcept
{
	NetDatagramAdmission result;
	if (datagram.size() <= CRC_SIZE) {
		result.Error = NetAdmissionError::DATAGRAM_TOO_SHORT;
		return(result);
	}

	result.Payload = datagram.subspan(CRC_SIZE);
	if (result.Payload.size() > payload_capacity) {
		result.Payload = {};
		result.Error = NetAdmissionError::DATAGRAM_TOO_LARGE;
		return(result);
	}

	std::memcpy(&result.WireCRC, datagram.data(), sizeof(result.WireCRC));
	if (result.WireCRC != Calculate_Network_Datagram_CRC(result.Payload)) {
		result.Payload = {};
		result.Error = NetAdmissionError::BAD_CRC;
	}
	return(result);
}


/// <summary>Validates a reliable-channel packet envelope.</summary>
NetConnectionAdmission Admit_Connection_Packet(std::span<std::byte const> packet, std::size_t header_size, std::size_t packet_capacity) noexcept
{
	NetConnectionAdmission result;
	if (header_size < COMMON_HEADER_SIZE || packet.size() < header_size) {
		result.Error = NetAdmissionError::HEADER_TOO_SHORT;
		return(result);
	}
	if (packet.size() > packet_capacity) {
		result.Error = NetAdmissionError::PACKET_TOO_LARGE;
		return(result);
	}

	std::memcpy(&result.Magic, packet.data(), sizeof(result.Magic));
	std::memcpy(&result.Code, packet.data() + sizeof(result.Magic), sizeof(result.Code));
	std::memcpy(&result.PacketID, packet.data() + sizeof(result.Magic) + sizeof(result.Code), sizeof(result.PacketID));
	if (result.Code >= static_cast<std::uint8_t>(NetPacketCode::COUNT)) {
		result.Error = NetAdmissionError::INVALID_PACKET_CODE;
		return(result);
	}

	result.Payload = packet.subspan(header_size);
	// Acknowledgements are header-only; data packet codes always carry application bytes.
	bool const ack_has_payload = result.Code == static_cast<std::uint8_t>(NetPacketCode::ACK) && !result.Payload.empty();
	bool const data_has_no_payload =
		(result.Code == static_cast<std::uint8_t>(NetPacketCode::DATA_ACK)
			|| result.Code == static_cast<std::uint8_t>(NetPacketCode::DATA_NOACK))
		&& result.Payload.empty();
	if (ack_has_payload || data_has_no_payload) {
		result.Payload = {};
		result.Error = NetAdmissionError::INVALID_PACKET_LENGTH;
	}
	return(result);
}


/// <summary>Checks that a caller can hold an admitted payload.</summary>
NetAdmissionError Validate_Network_Destination(std::span<std::byte const> payload, std::size_t destination_capacity) noexcept
{
	return(payload.size() <= destination_capacity ? NetAdmissionError::NONE : NetAdmissionError::DESTINATION_TOO_SMALL);
}


/// <summary>Returns a stable admission-error name.</summary>
char const * Net_Admission_Error_Name(NetAdmissionError error) noexcept
{
	switch (error) {
		case NetAdmissionError::NONE: return("none");
		case NetAdmissionError::DATAGRAM_TOO_SHORT: return("datagram too short");
		case NetAdmissionError::DATAGRAM_TOO_LARGE: return("datagram too large");
		case NetAdmissionError::BAD_CRC: return("bad datagram CRC");
		case NetAdmissionError::HEADER_TOO_SHORT: return("message header too short");
		case NetAdmissionError::PACKET_TOO_LARGE: return("message too large");
		case NetAdmissionError::INVALID_PACKET_CODE: return("invalid message code");
		case NetAdmissionError::INVALID_PACKET_LENGTH: return("invalid message length");
		case NetAdmissionError::DESTINATION_TOO_SMALL: return("destination too small");
		case NetAdmissionError::COUNT: break;
	}
	return("unknown admission error");
}
