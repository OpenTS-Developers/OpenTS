/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#pragma once

#include <cstddef>
#include <cstdint>
#include <span>


constexpr std::size_t NET_DATAGRAM_PAYLOAD_CAPACITY = 768;
constexpr std::size_t NET_PRIVATE_HEADER_SIZE = sizeof(std::uint16_t) + sizeof(std::uint8_t) + sizeof(std::uint32_t);
constexpr std::size_t NET_GLOBAL_HEADER_SIZE = NET_PRIVATE_HEADER_SIZE + sizeof(std::uint16_t);


enum class NetPacketCode : std::uint8_t
{
	DATA_ACK,
	DATA_NOACK,
	ACK,
	COUNT,
};


enum class NetAdmissionError
{
	NONE,
	DATAGRAM_TOO_SHORT,
	DATAGRAM_TOO_LARGE,
	BAD_CRC,
	HEADER_TOO_SHORT,
	PACKET_TOO_LARGE,
	INVALID_PACKET_CODE,
	INVALID_PACKET_LENGTH,
	DESTINATION_TOO_SMALL,
	COUNT,
};


struct NetDatagramAdmission
{
	NetAdmissionError Error = NetAdmissionError::NONE;
	std::uint32_t WireCRC = 0;
	std::span<std::byte const> Payload;

	bool Succeeded(void) const noexcept {return(Error == NetAdmissionError::NONE);}
};


struct NetConnectionAdmission
{
	NetAdmissionError Error = NetAdmissionError::NONE;
	std::uint16_t Magic = 0;
	std::uint8_t Code = 0;
	std::uint32_t PacketID = 0;
	std::span<std::byte const> Payload;

	bool Succeeded(void) const noexcept {return(Error == NetAdmissionError::NONE);}
};


std::uint32_t Calculate_Network_Datagram_CRC(std::span<std::byte const> payload) noexcept;

NetDatagramAdmission Admit_Network_Datagram(std::span<std::byte const> datagram, std::size_t payload_capacity = NET_DATAGRAM_PAYLOAD_CAPACITY) noexcept;

NetConnectionAdmission Admit_Connection_Packet(std::span<std::byte const> packet, std::size_t header_size, std::size_t packet_capacity) noexcept;

NetAdmissionError Validate_Network_Destination(std::span<std::byte const> payload, std::size_t destination_capacity) noexcept;

char const * Net_Admission_Error_Name(NetAdmissionError error) noexcept;
