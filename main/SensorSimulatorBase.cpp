/* ****************************************************************************
* Author    : Huwairis Ibnu Kabeer
* Company   : MrRISSU
* email     : huwairisibnukabeer777@gmail.com
* Mob No    : +91-9447504259
* File name : main.cpp
* ****************************************************************************/

#include "SensorSimulatorBase.hpp"
#include <iostream>
#include <random>
#include "esp_log.h"

SensorSimulatorBase::SensorSimulatorBase(std::string name) 
    : m_sensor_name(std::move(name)), 
    m_is_running(false), 
    m_period_ms(1000), 
    m_task_handle(nullptr)
{
    
}

SensorSimulatorBase::~SensorSimulatorBase()
{
    stopSimulation();
}

bool SensorSimulatorBase::startSimulation(uint32_t periodMs, uint32_t taskPriority)
{
    if (m_is_running) return false;
    
    m_period_ms = periodMs;
    m_is_running = true;

    BaseType_t result = xTaskCreate(
        &SensorSimulatorBase::taskWorker, // Worker function entry
        "sensor_sim_task",                // Task text identifier
        3072,                             // Stack depth in words
        this,                             // Task parameter pass-through
        taskPriority,                     // Priority level
        &m_task_handle                    // Saved task reference handle
    );

    if (result != pdPASS)
    {
        m_is_running = false;
        return false;
    }
    return true;
}

void SensorSimulatorBase::stopSimulation()
{
    if (!m_is_running)
        return;
    m_is_running = false;
    if (m_task_handle != nullptr)
    {
        vTaskDelete(m_task_handle);
        m_task_handle = nullptr;
    }
}

float SensorSimulatorBase::generateGaussianNoise(float mean, float stddev)
{
    // Static instances ensure the RNG isn't re-seeded on every function call
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::normal_distribution<float> dist(mean, stddev);
    return dist(gen);
}

void SensorSimulatorBase::taskWorker(void* pvParameters)
{
    auto* instance = static_cast<SensorSimulatorBase*>(pvParameters);
    TickType_t last_wake_time = xTaskGetTickCount();

    while (instance->m_is_running)
    {
        instance->generateNextSample();
        vTaskDelayUntil(&last_wake_time, pdMS_TO_TICKS(instance->m_period_ms));
    }
}