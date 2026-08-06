
/******************************************************************************
 * @file MQTT.hpp
 * @brief
 *
 * Author : Huwairis Ibnu Kabeer
 * License: MIT
 ******************************************************************************/

#ifndef MQTT_HPP
#define MQTT_HPP

/*==========================================================================*/
/* Includes                                                                 */
/*==========================================================================*/

/**
 * @file esp_transport.h
 * @brief Unified network transport abstraction layer for ESP-IDF.
 * 
 * WHY IT IS USED:
 * It decouples application logic (like HTTP or MQTT clients) from specific 
 * network protocols. This allows the same high-level code to send and receive 
 * data without worrying whether the underlying connection is raw TCP, 
 * secure TLS/SSL, or a WebSocket.
 * 
 * WHAT IT IS USED FOR:
 * - Providing standard API functions for connect, read, write, and close operations.
 * - Managing transport lists to easily switch between "http" and "https" schemes.
 * - Abstracting socket handles and handling underlying encryption seamlessly.
 */
#include "esp_transport.h"

/**
 * @file esp_transport_tcp.h
 * @brief TCP transport implementation for the ESP-IDF transport abstraction layer.
 * 
 * WHY IT IS USED:
 * It provides the concrete implementation for unencrypted, raw TCP socket 
 * communications. It plugs directly into the generic `esp_transport` interface 
 * so that higher-level clients can use standard TCP without managing raw 
 * BSD sockets manually.
 * 
 * WHAT IT IS USED FOR:
 * - Initializing and creating standard TCP transport handles (`esp_transport_tcp_init`).
 * - Setting core network parameters like destination IP, port, and interface names.
 * - Binding raw socket operations (connect, read, write, close) to the unified transport API.
 */
#include "esp_transport_tcp.h"

/**
 * @file esp_transport_ssl.h
 * @brief SSL/TLS transport implementation for the ESP-IDF transport abstraction layer.
 * 
 * WHY IT IS USED:
 * It adds a secure, encrypted layer (TLS/SSL) over raw TCP connections using mbedTLS. 
 * This allows higher-level clients (like HTTPS or secure MQTT) to communicate safely 
 * over the internet without needing to write complex, manual cryptographic handshake code.
 * 
 * WHAT IT IS USED FOR:
 * - Creating secure transport handles (`esp_transport_ssl_init`) for encrypted traffic.
 * - Configuring security credentials, including root certificates, client certificates, and private keys.
 * - Enabling features like ALPN (Application-Layer Protocol Negotiation), SNI (Server Name Indication), and PSK (Pre-Shared Keys).
 */
#include "esp_transport_ssl.h"

/**
 * @file esp_tls.h
 * @brief Simplified TLS/SSL wrapper layer for ESP-IDF.
 * 
 * WHY IT IS USED:
 * It acts as an easy-to-use, lightweight wrapper over the complex mbedTLS library. 
 * Instead of writing hundreds of lines of boilerplate code to handle handshakes, 
 * ciphersuites, and socket structures, it lets you establish a secure TLS 
 * connection with just a few function calls.
 * 
 * WHAT IT IS USED FOR:
 * - Opening secure connections directly using `esp_tls_conn_new_sync`.
 * - Configuring connection settings like certificates, timeouts, and SNI via `esp_tls_cfg_t`.
 * - Performing raw but secure network I/O (`esp_tls_conn_read` and `esp_tls_conn_write`).
 * - Fetching detailed TLS error codes for network debugging.
 */
#include "esp_tls.h"

// FreeRTOS components
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/timers.h"

// C++ Standard & ETL (Embedded Template Library)
#include <cstdint>
#include <etl/string_view.h>
#include <etl/unordered_map.h>
#include <etl/array.h>
// For zero-allocation callbacks
#include <etl/delegate.h>

/*==========================================================================*/
/* Macros                                                                   */
/*==========================================================================*/

// Adjust MAX_PAYLOAD_SIZE based on your SRAM constraints
constexpr size_t MAX_PAYLOAD_SIZE = 512; 
constexpr size_t MAX_TOPIC_LENGTH = 64;

/*==========================================================================*/
/* Enumerations                                                             */
/*==========================================================================*/

/*==========================================================================*/
/* Structures                                                               */
/*==========================================================================*/

typedef struct MqttMessage
{
    etl::array<char, MAX_TOPIC_LENGTH> topic;
    etl::array<uint8_t, MAX_PAYLOAD_SIZE> payload;
    size_t payload_length;
    uint8_t qos;
    bool retain;
}mqttMessage_t;

/*==========================================================================*/
/* Configuration / Lookup Tables                                            */
/*==========================================================================*/

/*==========================================================================*/
/* Classes                                                                  */
/*==========================================================================*/

/*==========================================================================*/
/* Public API                                                               */
/*==========================================================================*/

#endif