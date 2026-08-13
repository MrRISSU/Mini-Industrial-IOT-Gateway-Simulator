/******************************************************************************
 * @file DeviceSimulator.cpp
 * @brief Device Simulator implementation running on Core 1
 *
 * Author : Huwairis Ibnu Kabeer
 * License: MIT
 ******************************************************************************/

/*==========================================================================*/
/* Includes                                                                 */
/*==========================================================================*/

#include "DeviceSimulator.hpp"
#include "esp_log.h"
#include <random>
#include <cstring>

/*==========================================================================*/
/* Static Variables                                                         */
/*==========================================================================*/

static const char* TAG = "[DeviceSim]";

/*==========================================================================*/
/* Public Member Functions                                                  */
/*==========================================================================*/

DeviceSimulator::DeviceSimulator() :
    m_task_handle(nullptr),
    m_is_running(false),
    pTagsRegistry(nullptr)
{
    
}

DeviceSimulator::~DeviceSimulator()
{
    Destroy();
}

bool DeviceSimulator::Initialise(DeviceSimulConf_t& conf, TagsRegistry& registry)
{
    if (m_is_running)
    {
        return false;
    }

    // Store the configuration containing blocks and tags 
    deviceSimulConf = &conf;
    pTagsRegistry = &registry;

    // Verify if the device level simulation is enabled 
    if (!deviceSimulConf->enable)
    {
        ESP_LOGW(TAG, "Simulator is disabled in configuration.");
        return false;
    }

    m_is_running = true;

    // Pinning the simulator to Core 1 as requested for Application tasks
    BaseType_t result = xTaskCreatePinnedToCore(
        &DeviceSimulator::taskWorker, // Worker function entry 
        "dev_sim_task",               // Task text identifier 
        4096,                         // Stack depth in words
        this,                         // Task parameter pass-through 
        5,                            // Priority level 
        &m_task_handle,               // Saved task reference handle 
        1                             // Pin to Core 1
    );

    if (result != pdPASS)
    {
        m_is_running = false;
        ESP_LOGE(TAG, "Failed to create simulator task on Core 1");
        return false;
    }

    ESP_LOGI(TAG, "Device Simulator Initialised on Core 1 for: %s", deviceSimulConf->dscr);
    return true;
}

void DeviceSimulator::Destroy()
{
    if (!m_is_running)
    {
        return;
    }
    
    m_is_running = false;
    if (m_task_handle != nullptr)
    {
        vTaskDelete(m_task_handle);
        m_task_handle = nullptr;
    }
    ESP_LOGI(TAG, "Device Simulator Stopped.");
}

/*==========================================================================*/
/* Private Member Functions                                                 */
/*==========================================================================*/

void DeviceSimulator::taskWorker(void* pvParameters)
{
    auto* instance = static_cast<DeviceSimulator*>(pvParameters);
    TickType_t last_wake_time = xTaskGetTickCount();

    // Assuming the interval provided in the struct is in seconds (e.g., 5s) 
    const uint32_t interval_ms = instance->deviceSimulConf->interval * 1000;

    while (instance->m_is_running)
    {
        // Simulate tag and value
        instance->processSimulation();
        // Go to sleep and do not wake up until exactly interval_ms have passed since last_wake_time
        vTaskDelayUntil(&last_wake_time, pdMS_TO_TICKS(interval_ms));
    }
    
    // If it reached here it means the simulator is stopped
    // Safely destroy the current task and free up the memory
    vTaskDelete(NULL);
}

void DeviceSimulator::processSimulation()
{
    ESP_LOGI(TAG, "--- Simulating Device: %s ---", deviceSimulConf->dscr);

    // Iterate through all configured blocks 
    for (int b = 0; b < deviceSimulConf->blockCount; ++b)
    {
        if (b >= MAX_BLOCKS)
        {
            break; 
        }
        
        DeviceSimulBlock_t& block = deviceSimulConf->block[b];
        
        // Skip disabled blocks
        if (!block.enable)
        {
            continue;
        }

        // Iterate through all tags within the current block 
        for (int t = 0; t < block.tagCount; ++t)
        {
            if (t >= MAX_TAGS)
            {
                break;
            }
            
            DeviceSimulTag_t& tag = block.tag[t];
            
            // Generate simulated data based on dataType configuration 
            if (strncmp(tag.dataType, "f32", 3) == 0)
            {
                // Generate a base arbitrary value injected with random walk noise 
                float simulatedValue = 230.0f + generateGaussianNoise(0.0f, 2.5f);
                ESP_LOGI(TAG, "Block [%s] Tag [%s] = %.2f", block.dscr, tag.name, simulatedValue);

                if (pTagsRegistry != nullptr)
                {
                    // Call the templated Write function from TagsRegistry
                    pTagsRegistry->Write<float>(tag.name, simulatedValue);
                }
            }
        }
    }
}

float DeviceSimulator::generateGaussianNoise(float mean, float stddev)
{
    // Static instances ensure the RNG isn't re-seeded on every function call 
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::normal_distribution<float> dist(mean, stddev);
    return dist(gen);
}