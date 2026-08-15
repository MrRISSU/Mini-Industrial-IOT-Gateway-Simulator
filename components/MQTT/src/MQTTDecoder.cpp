/******************************************************************************
 * @file MQTTDecoder.cpp
 * @brief MQTT 5.0 Packet Decoder implementation.
 *
 * Author : Huwairis Ibnu Kabeer
 * License: MIT
 ******************************************************************************/

/*==========================================================================*/
/* Includes                                                                 */
/*==========================================================================*/

#include "MQTTDecoder.hpp"

/*==========================================================================*/
/* Namespace                                                                */
/*==========================================================================*/

namespace mqtt
{

/*==========================================================================*/
/* Public Member Functions                                                  */
/*==========================================================================*/

bool Decoder::DecodeFixedHeader(const std::uint8_t* buffer,
                                std::size_t length,
                                FixedHeader& outHeader,
                                std::size_t& bytesRead)
{
    bytesRead = 0;
    if (buffer == nullptr || length < 2)
    {
        return false;
    }

    std::size_t offset = 0;
    std::uint8_t firstByte = 0;
    if (!ReadByte(buffer, length, offset, firstByte))
    {
        return false;
    }

    outHeader.type = static_cast<PacketType>((firstByte >> 4) & 0x0F);
    outHeader.flags = firstByte & 0x0F;

    std::size_t remainingLen = 0;
    if (!ReadVariableByteInt(buffer, length, offset, remainingLen))
    {
        return false;
    }

    outHeader.remainingLength = remainingLen;
    bytesRead = offset;
    return true;
}

bool Decoder::Decode(const std::uint8_t* buffer, std::size_t length, Packet& outPacket)
{
    FixedHeader header;
    std::size_t headerBytesRead = 0;

    if (!DecodeFixedHeader(buffer, length, header, headerBytesRead))
    {
        return false;
    }

    if (length < headerBytesRead + header.remainingLength)
    {
        // Incomplete packet in buffer
        return false;
    }

    outPacket.type = header.type;
    std::size_t offset = headerBytesRead;

    switch (header.type)
    {
    case PacketType::CONNACK:
        return DecodeConnack(buffer, length, offset, outPacket);

    case PacketType::PUBLISH:
        return DecodePublish(buffer, length, offset, headerBytesRead, header, outPacket);

    case PacketType::SUBACK:
        return DecodeSuback(buffer, length, offset, headerBytesRead, header, outPacket);

    case PacketType::PUBACK:
    case PacketType::PUBREC:
    case PacketType::PUBREL:
    case PacketType::PUBCOMP:
    case PacketType::UNSUBACK:
        return DecodeAck(buffer, length, offset, outPacket);

    case PacketType::PINGRESP:
        outPacket.type = PacketType::PINGRESP;
        return true;

    case PacketType::DISCONNECT:
        outPacket.type = PacketType::DISCONNECT;
        if (header.remainingLength >= 1)
        {
            std::uint8_t rCode = 0;
            if (ReadByte(buffer, length, offset, rCode))
            {
                outPacket.reason_code = static_cast<ReasonCode>(rCode);
            }
        }
        return true;

    default:
        return false;
    }
}

/*==========================================================================*/
/* Private Member Functions                                                 */
/*==========================================================================*/

bool Decoder::DecodeConnack(const std::uint8_t* buffer, std::size_t length, std::size_t& offset, Packet& outPacket)
{
    std::uint8_t connectFlags = 0;
    if (!ReadByte(buffer, length, offset, connectFlags))
    {
        return false;
    }

    outPacket.connackData.isSessionPresent = (connectFlags & 0x01) != 0;

    std::uint8_t reasonCodeByte = 0;
    if (!ReadByte(buffer, length, offset, reasonCodeByte))
    {
        return false;
    }
    outPacket.connackData.reasonCode = static_cast<ReasonCode>(reasonCodeByte);
    outPacket.reason_code = outPacket.connackData.reasonCode;

    // Skip MQTT 5.0 Properties
    std::size_t propLen = 0;
    if (ReadVariableByteInt(buffer, length, offset, propLen))
    {
        offset += propLen;
    }

    return true;
}

bool Decoder::DecodePublish(const std::uint8_t* buffer, std::size_t length, std::size_t& offset, std::size_t headerBytesRead, const FixedHeader& header, Packet& outPacket)
{
    outPacket.dup = (header.flags & 0x08) != 0;
    outPacket.qos = static_cast<Qos>((header.flags >> 1) & 0x03);
    outPacket.retain = (header.flags & 0x01) != 0;

    // Topic Name
    if (!ReadString(buffer, length, offset, outPacket.topic.data(), kMaxTopicLength, outPacket.topic_length))
    {
        return false;
    }

    // Packet Identifier if QoS > 0
    if (outPacket.qos != Qos::QoS0)
    {
        if (!ReadUint16(buffer, length, offset, outPacket.packet_id))
        {
            return false;
        }
    }
    else
    {
        outPacket.packet_id = 0;
    }

    // Properties Length (MQTT 5.0)
    std::size_t propLen = 0;
    if (!ReadVariableByteInt(buffer, length, offset, propLen))
    {
        return false;
    }
    offset += propLen;

    // Extract Application Payload
    std::size_t totalPacketLen = headerBytesRead + header.remainingLength;
    std::size_t payloadBytes = (totalPacketLen > offset) ? (totalPacketLen - offset) : 0;

    return ReadBytes(buffer, offset + payloadBytes, offset, outPacket.payload.data(), kMaxPayloadSize, outPacket.payload_length);
}

bool Decoder::DecodeSuback(const std::uint8_t* buffer, std::size_t length, std::size_t& offset, std::size_t headerBytesRead, const FixedHeader& header, Packet& outPacket)
{
    if (!ReadUint16(buffer, length, offset, outPacket.subackData.packetId))
    {
        return false;
    }
    outPacket.packet_id = outPacket.subackData.packetId;

    // Properties Length
    std::size_t propLen = 0;
    if (!ReadVariableByteInt(buffer, length, offset, propLen))
    {
        return false;
    }
    offset += propLen;

    std::size_t totalPacketLen = headerBytesRead + header.remainingLength;
    outPacket.subackData.reasonCodeCount = 0;

    while (offset < totalPacketLen && offset < length && outPacket.subackData.reasonCodeCount < kMaxSubscriptions)
    {
        std::uint8_t rCode = 0;
        if (!ReadByte(buffer, length, offset, rCode))
        {
            return false;
        }
        outPacket.subackData.reasonCodes[outPacket.subackData.reasonCodeCount++] = static_cast<ReasonCode>(rCode);
    }

    return true;
}

bool Decoder::DecodeAck(const std::uint8_t* buffer, std::size_t length, std::size_t& offset, Packet& outPacket)
{
    if (!ReadUint16(buffer, length, offset, outPacket.ackData.packetId))
    {
        return false;
    }
    outPacket.packet_id = outPacket.ackData.packetId;

    std::uint8_t reasonCodeByte = 0;
    if (!ReadByte(buffer, length, offset, reasonCodeByte))
    {
        return false;
    }
    outPacket.ackData.reasonCode = static_cast<ReasonCode>(reasonCodeByte);
    outPacket.reason_code = outPacket.ackData.reasonCode;

    // Properties Length
    std::size_t propLen = 0;
    if (ReadVariableByteInt(buffer, length, offset, propLen))
    {
        offset += propLen;
    }

    return true;
}

bool Decoder::ReadByte(const std::uint8_t* buffer, std::size_t length, std::size_t& offset, std::uint8_t& outValue)
{
    if (buffer == nullptr || offset >= length)
    {
        return false;
    }
    outValue = buffer[offset++];
    return true;
}

bool Decoder::ReadUint16(const std::uint8_t* buffer, std::size_t length, std::size_t& offset, std::uint16_t& outValue)
{
    if (buffer == nullptr || offset + 2 > length)
    {
        return false;
    }
    outValue = (static_cast<std::uint16_t>(buffer[offset]) << 8) | static_cast<std::uint16_t>(buffer[offset + 1]);
    offset += 2;
    return true;
}

bool Decoder::ReadVariableByteInt(const std::uint8_t* buffer, std::size_t length, std::size_t& offset, std::size_t& outValue)
{
    outValue = 0;
    std::size_t multiplier = 1;
    std::uint8_t encodedByte = 0;

    do
    {
        if (buffer == nullptr || offset >= length)
        {
            return false;
        }
        encodedByte = buffer[offset++];
        outValue += (encodedByte & 127) * multiplier;
        if (multiplier > 128 * 128 * 128)
        {
            return false; // Malformed Variable Byte Integer
        }
        multiplier *= 128;
    } while ((encodedByte & 128) != 0);

    return true;
}

bool Decoder::ReadString(const std::uint8_t* buffer, std::size_t length, std::size_t& offset, char* outStr, std::size_t maxLen, std::size_t& outLen)
{
    std::uint16_t strLen = 0;
    if (!ReadUint16(buffer, length, offset, strLen))
    {
        return false;
    }

    if (offset + strLen > length || outStr == nullptr)
    {
        return false;
    }

    std::size_t copyLen = (strLen < maxLen) ? strLen : (maxLen - 1);
    for (std::size_t i = 0; i < copyLen; ++i)
    {
        outStr[i] = static_cast<char>(buffer[offset + i]);
    }
    outStr[copyLen] = '\0';
    outLen = copyLen;
    offset += strLen;
    return true;
}

bool Decoder::ReadBytes(const std::uint8_t* buffer, std::size_t length, std::size_t& offset, std::uint8_t* outData, std::size_t maxLen, std::size_t& outLen)
{
    if (buffer == nullptr || offset > length || outData == nullptr)
    {
        return false;
    }

    std::size_t bytesToRead = length - offset;
    std::size_t copyLen = (bytesToRead < maxLen) ? bytesToRead : maxLen;

    for (std::size_t i = 0; i < copyLen; ++i)
    {
        outData[i] = buffer[offset + i];
    }
    outLen = copyLen;
    offset += bytesToRead;
    return true;
}

} // namespace mqtt
