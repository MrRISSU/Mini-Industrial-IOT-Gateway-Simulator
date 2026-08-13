/******************************************************************************
 * @file main.cpp
 * @brief Mini Industrial IoT Gateway
 *
 * Author : Huwairis Ibnu Kabeer
 * License: MIT
 ******************************************************************************/

#include "BuildSettings.hpp"
#include "main.hpp"

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
    // Allocated on the heap to save app_main stack space
    NetworkManager* WiFi = new NetworkManager();
    if (WiFi->Initialize())
    {
        WiFi->connect("Technet", "Technet@23");
    }
    



    // Initialise Tag Registry
    TagsRegistry* tagsRegistry = new TagsRegistry();
    tagsRegistry->Initialize(50);




    // Initialise Device Simulators (Task runs on CORE - 1)
    DeviceSimulator* EnergyMeter = new DeviceSimulator();
    EnergyMeter->Initialise(EnergyMeterConf, *tagsRegistry);
    
    DeviceSimulator* DHT20 = new DeviceSimulator();
    DHT20->Initialise(DHT20Conf, *tagsRegistry);



    // Test TCP and TLS by connecting to example.com
    TcpTlsTest();

    /* =========================================================================
     * APPROACH 1: INFINITE LOOP (Currently Disabled)
     * =========================================================================
     * This approach keeps the app_main task alive forever by blocking it.
     * 
     * Drawback: It permanently hoards the ~4KB to 8KB of stack RAM allocated 
     * to the app_main task by ESP-IDF during boot. This memory cannot be used 
     * by your network or Modbus tasks, increasing the risk of memory exhaustion.
     */
    // while (true)
    // {
    //     // Blocks the task for 10 seconds, then wakes up just to block again
    //     vTaskDelay(pdMS_TO_TICKS(10000));
    // }

    /* =========================================================================
     * APPROACH 2: TASK DELETION (Active & Recommended)
     * =========================================================================
     * Passing NULL to vTaskDelete destroys the currently executing task (app_main).
     * 
     * Benefit: It immediately frees the app_main stack space back to the 
     * FreeRTOS heap. Because we instantiated our main components (WiFi, 
     * tagsRegistry, simulators) on the heap using pointers, they and their 
     * respective Core 0/1 tasks remain perfectly alive and operational.
     * 
     * WARNING: If any variables or objects were created as local stack variables 
     * instead of on the heap, they would get destroyed at this exact moment. 
     * If other modules (like your simulator tasks) were using references to 
     * that local memory, they would misbehave or instantly crash the gateway.
     */
    ESP_LOGI(TAG, "Initialization complete. Reclaiming app_main stack memory.");
    ESP_LOGW(TAG, "WARNING: app_main stack is being destroyed. Any local variables passed by reference to background tasks will now cause a crash!");
    vTaskDelete(NULL);
}


