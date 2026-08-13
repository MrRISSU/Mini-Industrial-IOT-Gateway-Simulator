/******************************************************************************
 * @file    TcpTransport.cpp
 * @brief   Implementation of the TCP transport.
 *
 * Implements connection management and common transport operations using the
 * ESP-IDF transport component.
 *
 * Author : Huwairis Ibnu Kabeer
 * License: MIT
 ******************************************************************************/

/*==========================================================================*/
/* Includes                                                                 */
/*==========================================================================*/

#include "TcpTransport.hpp"
#include "esp_log.h"

/*==========================================================================*/
/* Macros                                                                   */
/*==========================================================================*/

/*==========================================================================*/
/* Static Variables                                                         */
/*==========================================================================*/

static const char* TAG = "[TcpTransport]";

/*==========================================================================*/
/* Static Configuration / Lookup Tables                                     */
/*==========================================================================*/

/*==========================================================================*/
/* Static Helper Functions                                                  */
/*==========================================================================*/

/*==========================================================================*/
/* Public Member Functions                                                  */
/*==========================================================================*/

TcpTransport::TcpTransport()
{

}

TcpTransport::~TcpTransport()
{
    Destroy();
}

bool TcpTransport::Initialise()
{
    // Initialize the specific TCP transport handle at object creation.
    // This underlying handle is assigned to the 'transport_handle' pointer
    transport_handle = esp_transport_tcp_init();
    
    if (transport_handle == nullptr)
    {
        ESP_LOGE(TAG, "Failed to initialize TCP transport handle\r\n");
        return false;
    }
    return true;
}

bool TcpTransport::Destroy()
{
    // Clean up the memory allocated by esp_transport_tcp_init
    if (transport_handle != nullptr)
    {
        esp_transport_destroy(transport_handle);
        transport_handle = nullptr;
        return true;
    }
    return false;
}

bool TcpTransport::Connect(const char* host, std::uint16_t port, int timeout_ms)
{
    if (transport_handle == nullptr)
    {
        ESP_LOGE(TAG, "Cannot connect: Transport handle is null\r\n");
        return false;
    }

    ESP_LOGI(TAG, "Connecting to %s:%d...\r\n", host, port);

    // esp_transport_connect handles DNS resolution, socket creation, 
    // and the TCP 3-way handshake entirely under the hood.
    // It returns 0 (or a positive socket descriptor) on success, and < 0 on failure.
    int ret = esp_transport_connect(transport_handle, host, port, timeout_ms);
    
    if (ret < 0)
    {
        ESP_LOGE(TAG, "TCP connection failed (Error Code: %d)\r\n", ret);
        return false;
    }

    ESP_LOGI(TAG, "TCP connection established successfully\r\n");
    return true;
}

void TcpTransport::Disconnect()
{
    if (transport_handle != nullptr)
    {
        // esp_transport_close securely terminates the connection 
        // (sends TCP FIN or TLS close notify).
        // Crucially for zero-fragmentation: It closes the socket but does NOT 
        // destroy the handle's memory context. You can call Connect() again safely.
        esp_transport_close(transport_handle);
    }
}

int TcpTransport::Read(std::uint8_t* buffer, std::size_t len, int timeout_ms)
{
    if (transport_handle == nullptr)
    {
        // Return error if handle is uninitialized
        return -1;
    }

    // esp_transport_read handles the underlying poll()/select() logic 
    // to enforce your FreeRTOS timeout automatically. 
    // We use reinterpret_cast because ESP-IDF expects a char*, but some might use raw bytes.
    return esp_transport_read(transport_handle, reinterpret_cast<char*>(buffer), len, timeout_ms);
}

int TcpTransport::Write(const std::uint8_t* buffer, std::size_t len, int timeout_ms)
{
    if (transport_handle == nullptr)
    {
        // Return error if handle is uninitialized
        return -1;
    }

    // esp_transport_write sends the payload over the active socket.
    // Like Read(), it manages the timeout if the TCP window is full and blocking.
    // We use reinterpret_cast because ESP-IDF expects a char*, but some might use raw bytes.
    return esp_transport_write(transport_handle, reinterpret_cast<const char*>(buffer), len, timeout_ms);
}

/*==========================================================================*/
/* Private Member Functions                                                 */
/*==========================================================================*/