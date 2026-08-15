/******************************************************************************
 * @file MQTTEncoder.cpp
 * @brief MQTT 5.0 Packet Encoder implementation.
 *
 * Author : Huwairis Ibnu Kabeer
 * License: MIT
 ******************************************************************************/

/*==========================================================================*/
/* Includes                                                                 */
/*==========================================================================*/

#include "MQTTEncoder.hpp"

/*==========================================================================*/
/* Namespace                                                                */
/*==========================================================================*/

namespace mqtt
{

/*==========================================================================*/
/* Public Member Functions                                                  */
/*==========================================================================*/

Encoder::Encoder()
    : buffer_()
    , encodedLength_(0)
{
}

bool Encoder::Encode(const Packet& packet)
{
    encodedLength_ = 0;

    switch (packet.type)
    {
    case PacketType::CONNECT:
        return EncodeConnect(packet);

    case PacketType::PUBLISH:
        return EncodePublish(packet);

    case PacketType::SUBSCRIBE:
        return EncodeSubscribe(packet);

    case PacketType::PINGREQ:
        return EncodePingreq();

    case PacketType::PUBACK:
    case PacketType::PUBREC:
    case PacketType::PUBREL:
    case PacketType::PUBCOMP:
    case PacketType::UNSUBACK:
        return EncodeAck(packet);

    case PacketType::DISCONNECT:
        return EncodeDisconnect();

    default:
        return false;
    }
}

const std::uint8_t* Encoder::GetBuffer() const
{
    return buffer_.data();
}

std::size_t Encoder::GetEncodedLength() const
{
    return encodedLength_;
}

/*==========================================================================*/
/* Private Member Functions                                                 */
/*==========================================================================*/

bool Encoder::EncodeConnect(const Packet& packet)
{
    const auto& conn = packet.connectData;

    // Calculate Variable Header Length
    // Protocol Name ("MQTT"): 2 bytes len + 4 bytes string = 6
    // Protocol Level (0x05): 1 byte
    // Connect Flags: 1 byte
    // Keep Alive: 2 bytes
    // Properties Length: 1 byte (0x00 for no props)
    std::size_t varHeaderLen = 6 + 1 + 1 + 2 + 1;

    // Calculate Payload Length
    // Client ID: 2 bytes len + clientIdLen
    std::size_t payloadLen = 2 + conn.clientIdLen;

    // Connect Flags construction
    std::uint8_t connectFlags = 0;

    if (conn.isCleanStart)
    {
        connectFlags |= 0x02; // Bit 1: Clean Start
    }

    if (conn.hasWill)
    {
        connectFlags |= 0x04; // Bit 2: Will Flag
        connectFlags |= (static_cast<std::uint8_t>(conn.willQos) & 0x03) << 3; // Bits 4-3: Will QoS
        if (conn.isWillRetain)
        {
            connectFlags |= 0x20; // Bit 5: Will Retain
        }
        // Will Props Len (1 byte: 0x00) + Will Topic (2 + len) + Will Payload (2 + len)
        payloadLen += 1 + (2 + conn.willTopicLen) + (2 + conn.willPayloadLen);
    }

    if (conn.usernameLen > 0)
    {
        connectFlags |= 0x80; // Bit 7: Username Flag
        payloadLen += 2 + conn.usernameLen;
    }

    if (conn.passwordLen > 0)
    {
        connectFlags |= 0x40; // Bit 6: Password Flag
        payloadLen += 2 + conn.passwordLen;
    }

    std::size_t remainingLength = varHeaderLen + payloadLen;

    // Fixed Header: Packet Type 1 (CONNECT) -> 0x10, Flags -> 0x00
    if (!WriteByte(0x10))
    {
        return false;
    }

    if (!WriteVariableByteInt(remainingLength))
    {
        return false;
    }

    // --- Variable Header ---
    // Protocol Name
    if (!WriteString("MQTT", 4))
    {
        return false;
    }

    // Protocol Version (0x05 for MQTT 5.0)
    if (!WriteByte(kMqttVersion5))
    {
        return false;
    }

    // Connect Flags
    if (!WriteByte(connectFlags))
    {
        return false;
    }

    // Keep Alive
    if (!WriteUint16(conn.keepAliveSec))
    {
        return false;
    }

    // Properties Length (0x00)
    if (!WriteByte(0x00))
    {
        return false;
    }

    // --- Payload ---
    // Client ID
    if (!WriteString(conn.clientId.data(), conn.clientIdLen))
    {
        return false;
    }

    // Will
    if (conn.hasWill)
    {
        if (!WriteByte(0x00)) // Will Properties Length
        {
            return false;
        }
        if (!WriteString(conn.willTopic.data(), conn.willTopicLen))
        {
            return false;
        }
        if (!WriteUint16(static_cast<std::uint16_t>(conn.willPayloadLen)))
        {
            return false;
        }
        if (!WriteBytes(conn.willPayload.data(), conn.willPayloadLen))
        {
            return false;
        }
    }

    // Username
    if (conn.usernameLen > 0)
    {
        if (!WriteString(conn.username.data(), conn.usernameLen))
        {
            return false;
        }
    }

    // Password
    if (conn.passwordLen > 0)
    {
        if (!WriteString(conn.password.data(), conn.passwordLen))
        {
            return false;
        }
    }

    return true;
}

bool Encoder::EncodePublish(const Packet& packet)
{
    // Fixed Header Flags: (DUP << 3) | (QoS << 1) | Retain
    std::uint8_t flags = 0;
    if (packet.dup)
    {
        flags |= 0x08;
    }
    flags |= (static_cast<std::uint8_t>(packet.qos) & 0x03) << 1;
    if (packet.retain)
    {
        flags |= 0x01;
    }

    std::uint8_t firstByte = (static_cast<std::uint8_t>(PacketType::PUBLISH) << 4) | (flags & 0x0F);

    // Variable Header Length:
    // Topic Name (2 bytes len + topic_length)
    // Packet ID (2 bytes if QoS > 0)
    // Properties Length (1 byte: 0x00)
    std::size_t varHeaderLen = 2 + packet.topic_length + 1;
    if (packet.qos != Qos::QoS0)
    {
        varHeaderLen += 2;
    }

    std::size_t remainingLength = varHeaderLen + packet.payload_length;

    if (!WriteByte(firstByte))
    {
        return false;
    }

    if (!WriteVariableByteInt(remainingLength))
    {
        return false;
    }

    // Topic Name
    if (!WriteString(packet.topic.data(), packet.topic_length))
    {
        return false;
    }

    // Packet ID
    if (packet.qos != Qos::QoS0)
    {
        if (!WriteUint16(packet.packet_id))
        {
            return false;
        }
    }

    // Properties Length (0x00)
    if (!WriteByte(0x00))
    {
        return false;
    }

    // Payload
    if (!WriteBytes(packet.payload.data(), packet.payload_length))
    {
        return false;
    }

    return true;
}

bool Encoder::EncodeSubscribe(const Packet& packet)
{
    const auto& sub = packet.subscribeData;

    // Fixed Header Flags reserved as 0x02 for SUBSCRIBE (0x82)
    std::uint8_t firstByte = (static_cast<std::uint8_t>(PacketType::SUBSCRIBE) << 4) | 0x02;

    // Variable Header: Packet ID (2 bytes) + Properties Length (1 byte: 0x00)
    std::size_t varHeaderLen = 2 + 1;

    // Payload Length: sum of (2 + topicFilterLen + 1 byte options) for each filter
    std::size_t payloadLen = 0;
    for (std::size_t i = 0; i < sub.filterCount; ++i)
    {
        payloadLen += 2 + sub.filters[i].topicFilterLen + 1;
    }

    std::size_t remainingLength = varHeaderLen + payloadLen;

    if (!WriteByte(firstByte))
    {
        return false;
    }

    if (!WriteVariableByteInt(remainingLength))
    {
        return false;
    }

    // Packet ID
    if (!WriteUint16(sub.packetId))
    {
        return false;
    }

    // Properties Length (0x00)
    if (!WriteByte(0x00))
    {
        return false;
    }

    // Topic Filters
    for (std::size_t i = 0; i < sub.filterCount; ++i)
    {
        if (!WriteString(sub.filters[i].topicFilter.data(), sub.filters[i].topicFilterLen))
        {
            return false;
        }
        std::uint8_t subOptions = static_cast<std::uint8_t>(sub.filters[i].maxQos) & 0x03;
        if (!WriteByte(subOptions))
        {
            return false;
        }
    }

    return true;
}

bool Encoder::EncodePingreq()
{
    // Fixed Header: PINGREQ (0xC0), Remaining Length 0
    if (!WriteByte(0xC0))
    {
        return false;
    }
    return WriteByte(0x00);
}

bool Encoder::EncodeAck(const Packet& packet)
{
    std::uint8_t firstByte = (static_cast<std::uint8_t>(packet.type) << 4);
    if (packet.type == PacketType::PUBREL)
    {
        firstByte |= 0x02; // PUBREL flags reserved bit 1 = 1
    }

    // Variable Header: Packet ID (2 bytes) + Reason Code (1 byte) + Properties Length (1 byte: 0x00)
    std::size_t remainingLength = 2 + 1 + 1;

    if (!WriteByte(firstByte))
    {
        return false;
    }

    if (!WriteVariableByteInt(remainingLength))
    {
        return false;
    }

    if (!WriteUint16(packet.ackData.packetId))
    {
        return false;
    }

    if (!WriteByte(static_cast<std::uint8_t>(packet.ackData.reasonCode)))
    {
        return false;
    }

    return WriteByte(0x00); // Properties Length
}

bool Encoder::EncodeDisconnect()
{
    // Fixed Header: DISCONNECT (0xE0), Remaining Length 2 (Reason Code 0x00 + Props Len 0x00)
    if (!WriteByte(0xE0))
    {
        return false;
    }
    if (!WriteByte(0x02))
    {
        return false;
    }
    if (!WriteByte(0x00)) // Reason Code Normal Disconnection
    {
        return false;
    }
    return WriteByte(0x00); // Properties Length
}

bool Encoder::WriteByte(std::uint8_t value)
{
    if (encodedLength_ >= kMaxPacketSize)
    {
        return false;
    }
    buffer_[encodedLength_++] = value;
    return true;
}

bool Encoder::WriteUint16(std::uint16_t value)
{
    if (!WriteByte(static_cast<std::uint8_t>((value >> 8) & 0xFF)))
    {
        return false;
    }
    return WriteByte(static_cast<std::uint8_t>(value & 0xFF));
}

bool Encoder::WriteVariableByteInt(std::size_t value)
{
    do
    {
        std::uint8_t encodedByte = static_cast<std::uint8_t>(value % 128);
        value /= 128;
        if (value > 0)
        {
            encodedByte |= 0x80;
        }
        if (!WriteByte(encodedByte))
        {
            return false;
        }
    } while (value > 0);

    return true;
}

bool Encoder::WriteString(const char* str, std::size_t len)
{
    if (!WriteUint16(static_cast<std::uint16_t>(len)))
    {
        return false;
    }
    return WriteBytes(reinterpret_cast<const std::uint8_t*>(str), len);
}

bool Encoder::WriteBytes(const std::uint8_t* data, std::size_t len)
{
    if (encodedLength_ + len > kMaxPacketSize)
    {
        return false;
    }
    for (std::size_t i = 0; i < len; ++i)
    {
        buffer_[encodedLength_++] = data[i];
    }
    return true;
}

} // namespace mqtt
