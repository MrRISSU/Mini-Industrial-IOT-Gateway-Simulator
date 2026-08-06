/******************************************************************************
 * @file MQTTPacket.hpp
 * @brief Defines the statically sized data structures for MQTT messages, 
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
// For zero-allocation static arrays
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
 * @brief Statically sized MQTT message structure.
 * 
 * This struct is designed to be pushed and popped from FreeRTOS queues 
 * (like Tx Queue) entirely by value. Since it uses etl::array, bounded 
 * by config constants, it requires zero dynamic memory allocation.
 */
struct MqttMessage
{
    // --- Topic Data ---
    etl::array<char, config::MAX_TOPIC_LENGTH> topic;
    std::size_t topic_length;

    // --- Payload Data ---
    etl::array<std::uint8_t, config::MAX_PAYLOAD_SIZE> payload;
    std::size_t payload_length;

    // --- Metadata & Control Flags ---
    MqttQos       qos;       //
    bool          retain;    // True if this message should be retained by the broker
    bool          dup;       // Duplicate delivery flag (useful for QoS 1 & 2 retries)
    std::uint16_t packet_id; // Used internally for tracking QoS 1/2 acknowledgments
};

} // namespace mqtt