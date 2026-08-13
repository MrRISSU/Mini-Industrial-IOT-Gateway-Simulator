/******************************************************************************
 * @file    TlsTransport.cpp
 * @brief   Implementation of the TLS transport.
 *
 * Implements secure connection establishment and communication using the
 * ESP-IDF TLS transport component.
 *
 * Author : Huwairis Ibnu Kabeer
 * License: MIT
 ******************************************************************************/

/*==========================================================================*/
/* Includes                                                                 */
/*==========================================================================*/

#include "TlsTransport.hpp"
#include "esp_log.h"
#include <cstring>

/*==========================================================================*/
/* Macros                                                                   */
/*==========================================================================*/

/*==========================================================================*/
/* Static Variables                                                         */
/*==========================================================================*/

static const char* TAG = "TlsTransport";

/*==========================================================================*/
/* Static Configuration / Lookup Tables                                     */
/*==========================================================================*/

/*==========================================================================*/
/* Static Helper Functions                                                  */
/*==========================================================================*/

/*==========================================================================*/
/* Public Member Functions                                                  */
/*==========================================================================*/

TlsTransport::TlsTransport()
{

}

TlsTransport::~TlsTransport() 
{
    // Clean up the SSL handle and context when the object is destroyed
    Destroy();
}

bool TlsTransport::SetCert(const char* certPem) 
{
    if (certPem == nullptr)
    {
        ESP_LOGE(TAG, "Certificate is Empty\r\n");
        return false;
    }
    cert_pem = certPem;
    return true;
}

bool TlsTransport::Initialise() 
{
    // Initialize the new secure transport handle
    transport_handle = esp_transport_ssl_init();
    
    if (transport_handle == nullptr)
    {
        ESP_LOGE(TAG, "Failed to initialize SSL transport handle\r\n");
        return false;
    }

    // Apply the PEM certificate if one was provided
    if (cert_pem != nullptr)
    {
        // esp_transport_ssl_set_cert_data requires the string length of the PEM data
        // We add +1 to include the null-terminator for the mbedTLS parser
        esp_transport_ssl_set_cert_data(transport_handle, cert_pem, std::strlen(cert_pem) + 1);
    }
    else
    {
        ESP_LOGW(TAG, "No certificate provided. Connection may be rejected by strict brokers\r\n");
    }

    return true;
}

bool TlsTransport::Destroy() 
{
    if (transport_handle != nullptr)
    {
        esp_transport_destroy(transport_handle);
        transport_handle = nullptr;
        cert_pem = nullptr;
        return true;
    }
    return false;
}

/*==========================================================================*/
/* Private Member Functions                                                 */
/*==========================================================================*/
