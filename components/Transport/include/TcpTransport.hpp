/******************************************************************************
 * @file    TcpTransport.hpp
 * @brief   Plain TCP transport implementation using the ESP-IDF transport layer.
 *
 * Provides a TCP-based implementation of the Transport interface and shared
 * transport operations for TCP-derived transports.
 *
 * Author : Huwairis Ibnu Kabeer
 * License: MIT
 ******************************************************************************/

#pragma once

/*==========================================================================*/
/* Includes                                                                 */
/*==========================================================================*/

#include "Transport.hpp"
#include "esp_transport.h"
#include "transport_tcp.h"

/*==========================================================================*/
/* Macros                                                                   */
/*==========================================================================*/

/*==========================================================================*/
/* Enumerations                                                             */
/*==========================================================================*/

/*==========================================================================*/
/* Structures                                                               */
/*==========================================================================*/

/*==========================================================================*/
/* Global Variables                                                         */
/*==========================================================================*/

/*==========================================================================*/
/* Configuration / Lookup Tables                                            */
/*==========================================================================*/

/*==========================================================================*/
/* Classes                                                                  */
/*==========================================================================*/

/// @brief Unsecure TCP Transport Wrapper
class TcpTransport : public Transport
{
public:
    TcpTransport() = default;
    ~TcpTransport() override;
    
    // Only handles the TCP-specific initialization
    bool Initialise() override;
    bool Destroy() override;
    
    // Shared logic for TCP and TLS
    bool Connect(const char* host, std::uint16_t port, int timeout_ms) override;
    void Disconnect() override;
    int Read(std::uint8_t* buffer, std::size_t len, int timeout_ms) override;
    int Write(const std::uint8_t* buffer, std::size_t len, int timeout_ms) override;

protected:
    esp_transport_list_handle_t transport_list;
    esp_transport_handle_t      transport_handle;
};

/*==========================================================================*/
/* Public API                                                               */
/*==========================================================================*/

