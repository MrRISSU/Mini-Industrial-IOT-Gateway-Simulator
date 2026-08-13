/******************************************************************************
 * @file    TestTcpTls.cpp
 * @brief   Testing routines for both TCP and TLS transports.
 *
 * Provides standalone test functions to verify the unsecure and secure 
 * transport implementations against a public echo/web server.
 *
 * Author : Huwairis Ibnu Kabeer
 * License: MIT
 ******************************************************************************/

/*==========================================================================*/
/* Includes                                                                 */
/*==========================================================================*/

#include "TcpTransport.hpp"
#include "TlsTransport.hpp"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_log.h"
#include <cstring>

/*==========================================================================*/
/* Static Variables                                                         */
/*==========================================================================*/

static const char* TCP_TEST_TAG = "TcpTest";
static const char* TLS_TEST_TAG = "TlsTest";

// The working ECC Root CA for example.com
static const char* root_cert_pem = R"EOF(
-----BEGIN CERTIFICATE-----
MIICOjCCAcCgAwIBAgIQFAP1q/s3ixdAW+JDsqXRxDAKBggqhkjOPQQDAzBOMQsw
CQYDVQQGEwJVUzEYMBYGA1UECgwPU1NMIENvcnBvcmF0aW9uMSUwIwYDVQQDDBxT
U0wuY29tIFRMUyBFQ0MgUm9vdCBDQSAyMDIyMB4XDTIyMDgyNTE2MzM0OFoXDTQ2
MDgxOTE2MzM0N1owTjELMAkGA1UEBhMCVVMxGDAWBgNVBAoMD1NTTCBDb3Jwb3Jh
dGlvbjElMCMGA1UEAwwcU1NMLmNvbSBUTFMgRUNDIFJvb3QgQ0EgMjAyMjB2MBAG
ByqGSM49AgEGBSuBBAAiA2IABEUpNXP6wrgjzhR9qLFNoFs27iosU8NgCTWyJGYm
acCzldZdkkAZDsalE3D07xJRKF3nzL35PIXBz5SQySvOkkJYWWf9lCcQZIxPBLFN
SeR7T5v15wj4A4j3p8OSSxlUgaNjMGEwDwYDVR0TAQH/BAUwAwEB/zAfBgNVHSME
GDAWgBSJjy+j6CugFFR781a4Jl9nOAuc0DAdBgNVHQ4EFgQUiY8vo+groBRUe/NW
uCZfZzgLnNAwDgYDVR0PAQH/BAQDAgGGMAoGCCqGSM49BAMDA2gAMGUCMFXjIlbp
15IkWE8elDIPDAI2wv2sdDJO4fscgIijzPvX6yv/N33w7deedWo1dlJF4AIxAMeN
b0Igj762TVntd00pxCAgRWSGOlDGxK0tk/UYfXLtqc/ErFc2KAhl3zx5Zn6g6g==
-----END CERTIFICATE-----
)EOF";

/*==========================================================================*/
/* Public Test Functions                                                    */
/*==========================================================================*/

void TestTcpTransport()
{
    ESP_LOGI(TCP_TEST_TAG, "--- Starting TCP Transport Test ---");
    TcpTransport tcp;

    if (!tcp.Initialise())
    {
        ESP_LOGE(TCP_TEST_TAG, "TCP Initialization failed");
        return;
    }

    // Connect to port 80 for standard HTTP
    if (tcp.Connect("example.com", 80, 10000))
    {
        const char* get_req = "GET / HTTP/1.1\r\nHost: example.com\r\nConnection: close\r\n\r\n";
        
        ESP_LOGI(TCP_TEST_TAG, "Sending HTTP GET Request...");
        tcp.Write(reinterpret_cast<const uint8_t*>(get_req), std::strlen(get_req), 5000);

        uint8_t rx_buf[128];
        int bytes_read;
        ESP_LOGI(TCP_TEST_TAG, "Awaiting Response...");
        
        // Read in chunks until the server closes the connection
        while ((bytes_read = tcp.Read(rx_buf, sizeof(rx_buf) - 1, 5000)) > 0)
        {
            rx_buf[bytes_read] = '\0'; // Null-terminate to print as string
            ESP_LOGI(TCP_TEST_TAG, "%s", rx_buf);
        }
        
        ESP_LOGI(TCP_TEST_TAG, "Closing TCP connection");
        tcp.Disconnect();
    }
    else
    {
        ESP_LOGE(TCP_TEST_TAG, "Failed to connect to example.com via TCP");
    }

    // tcp.Destroy();
    ESP_LOGI(TCP_TEST_TAG, "--- TCP Transport Test Complete ---\r\n");
}

void TestTlsTransport()
{
    ESP_LOGI(TLS_TEST_TAG, "--- Starting TLS Transport Test ---");
    TlsTransport tls;
    
    tls.SetCert(root_cert_pem);
    
    if (!tls.Initialise())
    {
        ESP_LOGE(TLS_TEST_TAG, "TLS Initialization failed");
        return;
    }

    // Connect to port 443 for HTTPS
    if (tls.Connect("example.com", 443, 10000))
    {
        const char* get_req = "GET / HTTP/1.1\r\nHost: example.com\r\nConnection: close\r\n\r\n";
        
        ESP_LOGI(TLS_TEST_TAG, "Sending HTTPS GET Request...");
        tls.Write(reinterpret_cast<const uint8_t*>(get_req), std::strlen(get_req), 5000);

        uint8_t rx_buf[128];
        int bytes_read;
        ESP_LOGI(TLS_TEST_TAG, "Awaiting Secure Response...");
        
        // Read in chunks until the server closes the connection
        while ((bytes_read = tls.Read(rx_buf, sizeof(rx_buf) - 1, 5000)) > 0)
        {
            rx_buf[bytes_read] = '\0'; // Null-terminate to print as string
            ESP_LOGI(TLS_TEST_TAG, "%s", rx_buf);
        }
        
        ESP_LOGI(TLS_TEST_TAG, "Closing TLS connection");
        tls.Disconnect();
    }
    else
    {
        ESP_LOGE(TLS_TEST_TAG, "Failed to connect to example.com securely");
    }
    
    // tls.Destroy();
    ESP_LOGI(TLS_TEST_TAG, "--- TLS Transport Test Complete ---\r\n");
}

void TcpTestTask(void* pvParameters)
{
    ESP_LOGI(TCP_TEST_TAG, "TCP Test Task Started on Core %d", xPortGetCoreID());
    
    while (true)
    {
        TestTcpTransport();
        
        // Wait 15 seconds before the next request to avoid IP rate-limiting
        vTaskDelay(pdMS_TO_TICKS(15000));
    }
    
    vTaskDelete(NULL); 
}

void TlsTestTask(void* pvParameters)
{
    ESP_LOGI(TLS_TEST_TAG, "TLS Test Task Started on Core %d", xPortGetCoreID());
    
    // Offset the start time slightly so both tasks don't hit the network 
    // at the exact same millisecond
    vTaskDelay(pdMS_TO_TICKS(2000)); 

    while (true)
    {
        TestTlsTransport();
        
        // Wait 15 seconds before the next secure request
        vTaskDelay(pdMS_TO_TICKS(15000));
    }
    
    vTaskDelete(NULL); 
}

void TcpTlsTest()
{
    // =========================================================================
    // START TRANSPORT TESTS ON CORE 0
    // =========================================================================

    // Param 1: Task function
    // Param 2: Task name
    // Param 3: Stack size (8192 bytes to handle mbedTLS)
    // Param 4: Task parameters (NULL)
    // Param 5: Priority (5 is standard for network tasks)
    // Param 6: Task handle (NULL)
    // Param 7: Core ID (0)

    xTaskCreatePinnedToCore(TcpTestTask, "TCP_Task", 4096, NULL, 5, NULL, 0);
    xTaskCreatePinnedToCore(TlsTestTask, "TLS_Task", 8192, NULL, 5, NULL, 0);
}
