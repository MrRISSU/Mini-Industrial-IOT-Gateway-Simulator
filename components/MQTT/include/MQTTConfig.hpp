
/******************************************************************************
 * @file
 * @brief
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

/*==========================================================================*/
/* Namespace                                                                */
/*==========================================================================*/

namespace mqtt
{

/*==========================================================================*/
/* constant Expressions                                                     */
/*==========================================================================*/

// Compile-Time Memory Constraints
constexpr size_t MAX_TOPIC_LENGTH    = 128;
constexpr size_t MAX_PAYLOAD_SIZE    = 1024;
constexpr size_t MAX_HEADER_OVERHEAD = 32; 
constexpr size_t MAX_PACKET_SIZE     = MAX_TOPIC_LENGTH + MAX_PAYLOAD_SIZE + MAX_HEADER_OVERHEAD;

constexpr uint32_t RX_TASK_STACK_SIZE = 4096;
constexpr uint32_t TX_TASK_STACK_SIZE = 3072;
constexpr size_t   TX_QUEUE_LENGTH    = 10;
constexpr size_t   MAX_SUBSCRIPTIONS  = 20;

/*==========================================================================*/
/* Structures                                                               */
/*==========================================================================*/

// Runtime Configuration Structure
struct MQTTClientConf_t {
    const char* dscr;           // Description or Client ID
    uint8_t     enable;         // 1 to start on boot, 0 to hold
    const char* broker_uri;
    uint16_t    port;
    uint16_t    keep_alive_sec;
    int         timeout_ms;
    uint8_t     rx_task_priority;
    uint8_t     tx_task_priority;
    bool        clean_session;
};

/*==========================================================================*/
/* Global Variables                                                         */
/*==========================================================================*/

// Declare the global instance so other files can see it
extern MQTTClientConf_t DefaultMQTTConf;

} // namespace mqtt
