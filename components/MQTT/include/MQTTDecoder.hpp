
/******************************************************************************
 * @file MQTTDecoder.hpp
 * @brief MQTT 5.0 Packet Decoder class declaration.
 *
 * Author : Huwairis Ibnu Kabeer
 * License: MIT
 ******************************************************************************/

#pragma once

/*==========================================================================*/
/* Includes                                                                 */
/*==========================================================================*/

#include <cstdint>
#include <cstddef>

#include "MQTTConfig.hpp"
#include "MQTTPacket.hpp"

/*==========================================================================*/
/* Namespace                                                                */
/*==========================================================================*/

namespace mqtt
{

/*==========================================================================*/
/* Classes                                                                  */
/*==========================================================================*/

/**
 * @brief Zero-allocation decoder for parsing raw byte streams into MQTT 5.0 packets.
 */
class Decoder
{
public:
    Decoder() = default;
    ~Decoder() = default;

    /**
     * @brief Parses an MQTT Fixed Header from a raw byte buffer.
     * @param buffer Pointer to raw input bytes.
     * @param length Total length available in buffer.
     * @param outHeader Fixed header structure to populate.
     * @param bytesRead Output number of bytes consumed by fixed header.
     * @return true if fixed header parsed successfully, false otherwise.
     */
    bool DecodeFixedHeader(const std::uint8_t* buffer,
                           std::size_t length,
                           FixedHeader& outHeader,
                           std::size_t& bytesRead);

    /**
     * @brief Decodes a full MQTT packet from a raw byte buffer.
     * @param buffer Pointer to raw input bytes.
     * @param length Total length available in buffer.
     * @param outPacket Target packet structure to populate.
     * @return true if decoding succeeded, false if data is malformed or incomplete.
     */
    bool Decode(const std::uint8_t* buffer, std::size_t length, Packet& outPacket);

private:
    // Packet specific payload decoders
    bool DecodeConnack(const std::uint8_t* buffer, std::size_t length, std::size_t& offset, Packet& outPacket);
    bool DecodePublish(const std::uint8_t* buffer, std::size_t length, std::size_t& offset, std::size_t headerBytesRead, const FixedHeader& header, Packet& outPacket);
    bool DecodeSuback(const std::uint8_t* buffer, std::size_t length, std::size_t& offset, std::size_t headerBytesRead, const FixedHeader& header, Packet& outPacket);
    bool DecodeAck(const std::uint8_t* buffer, std::size_t length, std::size_t& offset, Packet& outPacket);

    // Memory-safe stream reader helpers
    bool ReadByte(const std::uint8_t* buffer, std::size_t length, std::size_t& offset, std::uint8_t& outValue);
    bool ReadUint16(const std::uint8_t* buffer, std::size_t length, std::size_t& offset, std::uint16_t& outValue);
    bool ReadVariableByteInt(const std::uint8_t* buffer, std::size_t length, std::size_t& offset, std::size_t& outValue);
    bool ReadString(const std::uint8_t* buffer, std::size_t length, std::size_t& offset, char* outStr, std::size_t maxLen, std::size_t& outLen);
    bool ReadBytes(const std::uint8_t* buffer, std::size_t length, std::size_t& offset, std::uint8_t* outData, std::size_t maxLen, std::size_t& outLen);
};

} // namespace mqtt

