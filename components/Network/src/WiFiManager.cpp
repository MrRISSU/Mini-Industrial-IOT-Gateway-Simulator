/******************************************************************************
 * @file    WiFiManager.cpp
 * @brief   WiFi manager implementation for handling WiFi connectivity.
 *
 * Author : Huwairis Ibnu Kabeer
 * License: MIT
 ******************************************************************************/

#include "WiFiManager.hpp"
#include "esp_log.h"
#include "esp_netif.h"
#include <cstring>

static const char* TAG = "WiFiManager";

NetworkManager::NetworkManager() : instance_any_id(nullptr), instance_got_ip(nullptr)
{
}

NetworkManager::~NetworkManager()
{
    disconnect();
    esp_wifi_deinit();
}

bool NetworkManager::Initialize()
{
    // Initialize the underlying TCP/IP stack
    esp_err_t ret = esp_netif_init();
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to initialize TCP/IP stack (%s)\r\n", esp_err_to_name(ret));
        return false;
    }
    
    // Create default event loop
    ret = esp_event_loop_create_default();
    if (ret != ESP_OK && ret != ESP_ERR_INVALID_STATE)
    {
        ESP_LOGE(TAG, "Failed to create event loop (%s)\r\n", esp_err_to_name(ret));
        return false;
    }
    
    // Creates default WIFI STA. In case of any init error this API aborts.
    esp_netif_create_default_wifi_sta();
    
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    // Initialize WiFi Allocate resource for WiFi driver
    // such as WiFi control structure, RX/TX buffer, WiFi NVS structure etc.
    // This WiFi also starts WiFi task
    ret = esp_wifi_init(&cfg);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to initialize WiFi (%s)\r\n", esp_err_to_name(ret));
        return false;
    }
    
    return true;
}

void NetworkManager::connect(const std::string& ssid, const std::string& password)
{
    // Register event handlers for WiFi and IP events
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &event_handler,
                                                        this,
                                                        &instance_any_id));
                                                        
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                        IP_EVENT_STA_GOT_IP,
                                                        &event_handler,
                                                        this,
                                                        &instance_got_ip));

    wifi_config_t wifi_config = {};
    
    // Copy SSID and Password into the configuration struct safely
    std::strncpy(reinterpret_cast<char*>(wifi_config.sta.ssid), ssid.c_str(), sizeof(wifi_config.sta.ssid) - 1);
    std::strncpy(reinterpret_cast<char*>(wifi_config.sta.password), password.c_str(), sizeof(wifi_config.sta.password) - 1);
    
    // Set authentication mode based on whether a password was provided
    if (password.length() > 0)
    {
        wifi_config.sta.threshold.authmode = WIFI_AUTH_WPA2_PSK;
    }
    else
    {
        wifi_config.sta.threshold.authmode = WIFI_AUTH_OPEN;
    }

    // Set mode to Station, apply config, and start WiFi
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "WiFi initialization finished. Connecting to %s...", ssid.c_str());
}

void NetworkManager::disconnect()
{
    esp_wifi_disconnect();
    esp_wifi_stop();
    
    // Unregister events to prevent memory leaks if the object is destroyed
    if (instance_any_id)
    {
        esp_event_handler_instance_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, instance_any_id);
    }
    if (instance_got_ip)
    {
        esp_event_handler_instance_unregister(IP_EVENT, IP_EVENT_STA_GOT_IP, instance_got_ip);
    }
}

void NetworkManager::event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START)
    {
        // WiFi driver started, attempt connection
        esp_wifi_connect();
    } 
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED)
    {
        // Basic auto-reconnect feature
        ESP_LOGW(TAG, "Disconnected from AP, retrying...");
        esp_wifi_connect();
    } 
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP)
    {
        // Successfully connected and received an IP address
        ip_event_got_ip_t* event = reinterpret_cast<ip_event_got_ip_t*>(event_data);
        ESP_LOGI(TAG, "Got IP Address: " IPSTR, IP2STR(&event->ip_info.ip));
    }
}
