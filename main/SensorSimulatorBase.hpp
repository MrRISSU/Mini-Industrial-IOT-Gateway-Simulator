/* ****************************************************************************
* Author    : Huwairis Ibnu Kabeer
* Company   : MrRISSU
* email     : huwairisibnukabeer777@gmail.com
* Mob No    : +91-9447504259
* File name : main.cpp
* ****************************************************************************/

#pragma once

#include <string>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// Base abstract class for all sensor simulators
class SensorSimulatorBase
{
public:
    // Explicitly delete copy constructors to prevent object slicing
    SensorSimulatorBase(const SensorSimulatorBase&) = delete;
    SensorSimulatorBase& operator=(const SensorSimulatorBase&) = delete;

    // Virtual destructor ensures proper cleanup of derived objects
    virtual ~SensorSimulatorBase();

    // Core Lifecycle Interface Methods
    virtual bool initialize() = 0;
    virtual void reset() = 0;
    virtual float read() = 0;

    // Starts background updates using native ESP-IDF FreeRTOS tasks
    bool startSimulation(uint32_t periodMs, uint32_t taskPriority = 5);

    // Safe termination wrapper for background simulation tasks
    void stopSimulation();

protected:
    // Protected constructor to enforce instantiation via derived instances only
    explicit SensorSimulatorBase(std::string name);

    // Pure virtual method handling unique mathematics for generation logic
    virtual void generateNextSample() = 0;

    // Standard helper for injecting pseudo-random noise variations
    float generateGaussianNoise(float mean, float stddev);

    std::string m_sensor_name;
    bool m_is_running;
    uint32_t m_period_ms;

private:
    TaskHandle_t m_task_handle;

    // Static FreeRTOS bridge to member functions
    static void taskWorker(void* pvParameters);
};