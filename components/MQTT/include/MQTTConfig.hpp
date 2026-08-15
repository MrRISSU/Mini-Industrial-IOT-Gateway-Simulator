
/******************************************************************************
 * @file MQTTConfig.hpp
 * @brief Configuration constants and runtime configuration structure for MQTT 5.0.
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

#include "MQTTTypes.hpp"

/*==========================================================================*/
/* Namespace                                                                */
/*==========================================================================*/

namespace mqtt
{

/*==========================================================================*/
/* Constant Expressions                                                     */
/*==========================================================================*/

// Compile-Time Memory Constraints
constexpr std::size_t kMaxTopicLength        = 128;
constexpr std::size_t kMaxPayloadSize        = 1024;
constexpr std::size_t kMaxHeaderOverhead     = 32;
constexpr std::size_t kMaxPacketSize         = kMaxTopicLength + kMaxPayloadSize + kMaxHeaderOverhead;

constexpr std::size_t kMaxClientIdLength     = 64;
constexpr std::size_t kMaxUsernameLength     = 64;
constexpr std::size_t kMaxPasswordLength     = 64;
constexpr std::size_t kMaxWillTopicLength    = 128;
constexpr std::size_t kMaxWillPayloadSize    = 256;

constexpr std::uint32_t kRxTaskStackSize_Bytes = 4096;
constexpr std::uint32_t kTxTaskStackSize_Bytes = 3072;
constexpr std::size_t   kTxQueueLength         = 10;
constexpr std::size_t   kMaxSubscriptions      = 20;

constexpr std::uint8_t  kMqttVersion5          = 0x05;

// Legacy Compatibility Aliases
constexpr std::size_t MAX_TOPIC_LENGTH    = kMaxTopicLength;
constexpr std::size_t MAX_PAYLOAD_SIZE    = kMaxPayloadSize;
constexpr std::size_t MAX_HEADER_OVERHEAD = kMaxHeaderOverhead;
constexpr std::size_t MAX_PACKET_SIZE     = kMaxPacketSize;
constexpr std::size_t RX_TASK_STACK_SIZE  = kRxTaskStackSize_Bytes;
constexpr std::size_t TX_TASK_STACK_SIZE  = kTxTaskStackSize_Bytes;
constexpr std::size_t TX_QUEUE_LENGTH     = kTxQueueLength;
constexpr std::size_t MAX_SUBSCRIPTIONS   = kMaxSubscriptions;

/*==========================================================================*/
/* Structures                                                               */
/*==========================================================================*/

/**
 * @brief Runtime Configuration Structure for MQTT 5.0 Client.
 */
struct Config
{
    const char* description;
    bool isEnabled;
    const char* brokerUri;
    std::uint16_t port;
    std::uint16_t keepAliveSec;
    int timeoutMs;
    std::uint8_t rxTaskPriority;
    std::uint8_t txTaskPriority;
    bool isCleanStart;

    // --- Authentication & Identity (Static Storage) ---
    etl::array<char, kMaxClientIdLength> clientId;
    std::size_t clientIdLen;

    etl::array<char, kMaxUsernameLength> username;
    std::size_t usernameLen;

    etl::array<char, kMaxPasswordLength> password;
    std::size_t passwordLen;

    // --- Quality of Service Defaults ---
    Qos defaultQos;

    // --- Last Will & Testament (LWT) ---
    bool hasWill;
    etl::array<char, kMaxWillTopicLength> willTopic;
    std::size_t willTopicLen;

    etl::array<std::uint8_t, kMaxWillPayloadSize> willPayload;
    std::size_t willPayloadLen;

    Qos willQos;
    bool isWillRetain;
};

using MQTTClientConf_t = Config;

/*==========================================================================*/
/* Global Variables                                                         */
/*==========================================================================*/

extern Config DefaultConf;

} // namespace mqtt

