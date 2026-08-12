/* ****************************************************************************
* Author    : Huwairis Ibnu Kabeer
* Company   : MrRISSU
* email     : huwairisibnukabeer777@gmail.com
* Mob No    : +91-9447504259
* File name : network_manager.hpp
* ****************************************************************************/
/* network_manager.hpp */
#pragma once

#include <string>
#include "esp_wifi.h"
#include "esp_event.h"

class NetworkManager
{
public:
    NetworkManager();
    ~NetworkManager();

    // Initializes the network stack. Call this inside app_main().
    bool Initialize();

    // Connects to the specified Access Point
    void connect(const std::string& ssid, const std::string& password);
    
    // Disconnects and stops the WiFi driver
    void disconnect();

private:
    static void event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data);
    
    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;
};

