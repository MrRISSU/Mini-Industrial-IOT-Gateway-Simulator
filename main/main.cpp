/******************************************************************************
 * @file main.cpp
 * @brief Mini Industrial IoT Gateway
 *
 * Author : Huwairis Ibnu Kabeer
 * License: MIT
 ******************************************************************************/

#include "nvs_flash.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_log.h"

#include "DeviceSimulator.hpp"
#include "sample_conf.hpp"

#include "TagsRegister.hpp"

#include "network_manager.hpp"

static const char* TAG = "main";


extern "C" void app_main()
{
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);



    // Initialise WIFI (Task runs on CORE - 0)
    NetworkManager WiFi;
    if (WiFi.Initialize())
    {
        WiFi.connect("Technet", "Technet@23");
    }
    


    // Initialise Tag Registry
    TagsRegistry tagsRegistry;
    tagsRegistry.Initialize(50);



    // Initialise Device Simulators (Task runs on CORE - 1)
    DeviceSimulator EnergyMeter;
    EnergyMeter.Initialise(EnergyMeterConf, tagsRegistry);
    DeviceSimulator DHT20;
    DHT20.Initialise(DHT20Conf, tagsRegistry);



    // Main task can now go to sleep or do other things
    while (true)
    {
        vTaskDelay(pdMS_TO_TICKS(10000));
    }
}


