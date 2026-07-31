/* ****************************************************************************
* Author    : Huwairis Ibnu Kabeer
* Company   : MrRISSU
* email     : huwairisibnukabeer777@gmail.com
* Mob No    : +91-9447504259
* File name : main.cpp
* ****************************************************************************/

#include "TempSensorSimulator.hpp"
#include "esp_log.h"

static const char* TAG = "TempSim";

TemperatureSimulator::TemperatureSimulator(std::string name) 
    : SensorSimulatorBase(std::move(name)), 
      m_current_temperature(25.0f)
{
    // Default start temp
}

bool TemperatureSimulator::initialize()
{
    ESP_LOGI(TAG, "Initializing %s", m_sensor_name.c_str());
    m_current_temperature = 25.0f;
    return true;
}

void TemperatureSimulator::reset()
{
    m_current_temperature = 25.0f;
}

void TemperatureSimulator::generateNextSample()
{
    // Add some random walk noise to the temperature
    m_current_temperature += generateGaussianNoise(0.0f, 0.5f);
    ESP_LOGI(TAG, "[%s: %.2f C]", m_sensor_name.c_str(), m_current_temperature);
}