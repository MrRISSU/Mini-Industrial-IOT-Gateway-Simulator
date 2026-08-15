/******************************************************************************
 * @file MQTTPacket.hpp
 * @brief Defines statically sized data structures for MQTT 5.0 packets, 
 *        designed for safe passage through FreeRTOS queues without the heap.
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
#include "MQTTTypes.hpp"

/*==========================================================================*/
/* Namespace                                                                */
/*==========================================================================*/

namespace mqtt
{

/*==========================================================================*/
/* Structures                                                               */
/*==========================================================================*/

/**
 * @brief Decoded MQTT Fixed Header.
 */
struct FixedHeader
{
    PacketType type;
    std::uint8_t flags;
    std::size_t remainingLength;
};

/**
 * @brief Static payload for CONNECT packet (MQTT 5.0).
 */
struct ConnectPacketPayload
{
    bool isCleanStart;
    std::uint16_t keepAliveSec;

    etl::array<char, kMaxClientIdLength> clientId;
    std::size_t clientIdLen;

    etl::array<char, kMaxUsernameLength> username;
    std::size_t usernameLen;

    etl::array<char, kMaxPasswordLength> password;
    std::size_t passwordLen;

    bool hasWill;
    etl::array<char, kMaxWillTopicLength> willTopic;
    std::size_t willTopicLen;

    etl::array<std::uint8_t, kMaxWillPayloadSize> willPayload;
    std::size_t willPayloadLen;

    Qos willQos;
    bool isWillRetain;
};

/**
 * @brief Static payload for CONNACK packet (MQTT 5.0).
 */
struct ConnackPacketPayload
{
    bool isSessionPresent;
    ReasonCode reasonCode;
};

/**
 * @brief Single topic filter item in a SUBSCRIBE request.
 */
struct SubscribeFilter
{
    etl::array<char, kMaxTopicLength> topicFilter;
    std::size_t topicFilterLen;
    Qos maxQos;
};

/**
 * @brief Static payload for SUBSCRIBE packet.
 */
struct SubscribePacketPayload
{
    std::uint16_t packetId;
    etl::array<SubscribeFilter, kMaxSubscriptions> filters;
    std::size_t filterCount;
};

/**
 * @brief Static payload for SUBACK packet.
 */
struct SubackPacketPayload
{
    std::uint16_t packetId;
    etl::array<ReasonCode, kMaxSubscriptions> reasonCodes;
    std::size_t reasonCodeCount;
};

/**
 * @brief Static payload for acknowledgment packets (PUBACK, PUBREC, PUBREL, PUBCOMP, UNSUBACK).
 */
struct AckPacketPayload
{
    std::uint16_t packetId;
    ReasonCode reasonCode;
};

/**
 * @brief Static payload for PUBLISH packet.
 */
struct PublishPacketPayload
{
    etl::array<char, kMaxTopicLength> topic;
    std::size_t topicLength;

    etl::array<std::uint8_t, kMaxPayloadSize> payload;
    std::size_t payloadLength;

    Qos qos;
    bool isRetain;
    bool isDup;
    std::uint16_t packetId;
};

/**
 * @brief Statically sized MQTT message container structure.
 * 
 * Designed to be pushed and popped from FreeRTOS queues 
 * (like Tx/Rx Queues) entirely by value with zero dynamic memory allocation.
 */
struct Packet
{
    PacketType type;

    // --- Topic Data ---
    etl::array<char, kMaxTopicLength> topic;
    std::size_t topic_length;

    // --- Payload Data ---
    etl::array<std::uint8_t, kMaxPayloadSize> payload;
    std::size_t payload_length;

    // --- Metadata & Control Flags ---
    Qos qos;
    bool retain;
    bool dup;
    std::uint16_t packet_id;
    ReasonCode reason_code;

    // --- Specific Control Packet Payloads ---
    ConnectPacketPayload connectData;
    ConnackPacketPayload connackData;
    SubscribePacketPayload subscribeData;
    SubackPacketPayload subackData;
    AckPacketPayload ackData;
};

} // namespace mqtt