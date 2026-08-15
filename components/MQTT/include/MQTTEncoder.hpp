
/******************************************************************************
 * @file MQTTEncoder.hpp
 * @brief MQTT 5.0 Packet Encoder class declaration.
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
#include <etl/array.h>

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
 * @brief Zero-allocation encoder for serializing MQTT 5.0 packets into byte buffers.
 */
class Encoder
{
public:
    Encoder();
    ~Encoder() = default;

    /**
     * @brief Serializes an MQTT packet into the internal static buffer.
     * @param packet Reference to the packet to encode.
     * @return true if serialization succeeded, false if buffer exceeded max capacity.
     */
    bool Encode(const Packet& packet);

    /**
     * @brief Returns a pointer to the raw encoded byte buffer.
     * @return Pointer to encoded byte stream.
     */
    const std::uint8_t* GetBuffer() const;

    /**
     * @brief Returns the length of the encoded packet in bytes.
     * @return Encoded byte count.
     */
    std::size_t GetEncodedLength() const;

private:
    // Static internal buffer for zero dynamic allocation
    etl::array<std::uint8_t, kMaxPacketSize> buffer_;
    std::size_t encodedLength_;

    // Specific packet encoders
    bool EncodeConnect(const Packet& packet);
    bool EncodePublish(const Packet& packet);
    bool EncodeSubscribe(const Packet& packet);
    bool EncodePingreq();
    bool EncodeAck(const Packet& packet);
    bool EncodeDisconnect();

    // Field serialization helpers
    bool WriteByte(std::uint8_t value);
    bool WriteUint16(std::uint16_t value);
    bool WriteVariableByteInt(std::size_t value);
    bool WriteString(const char* str, std::size_t len);
    bool WriteBytes(const std::uint8_t* data, std::size_t len);
};

} // namespace mqtt

