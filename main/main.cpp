/* ****************************************************************************
* Author    : Huwairis Ibnu Kabeer
* Company   : MrRISSU
* email     : huwairisibnukabeer777@gmail.com
* Mob No    : +91-9447504259
* File name : main.cpp
* ****************************************************************************/

#include "TempSensorSimulator.hpp"
#include "TagsRegister.hpp"
#include "network_manager.hpp"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nvs_flash.h"
#include "esp_log.h"

static const char* TAG = "main";


extern "C" void app_main()
{
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    NetworkManager WiFi;

    TemperatureSimulator engineTemp("Engine_Temp_1");

    TagsRegistry tagsRegistry;

    if (WiFi.Initialize())
    {
        WiFi.connect("Technet", "Technet@23");
    }

    if (engineTemp.initialize())
    {
        // Run at 1000ms intervals with priority 5
        engineTemp.startSimulation(1000, 5); 
    }

    tagsRegistry.Initialize(50);

    // Main task can now go to sleep or do other things
    while (true)
    {
        float temp = engineTemp.read();
        tagsRegistry.Write<float>("temperature", temp);
        temp = tagsRegistry.Read<float>("temperature");
        ESP_LOGI(TAG, "temperature: %.3f\r\n", temp);
        vTaskDelay(pdMS_TO_TICKS(10000));
    }
}


