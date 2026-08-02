/* ****************************************************************************
* Author    : Huwairis Ibnu Kabeer
* Company   : MrRISSU
* email     : huwairisibnukabeer777@gmail.com
* Mob No    : +91-9447504259
* File name : main.cpp
* ****************************************************************************/

#pragma once
#include "SensorSimulatorBase.hpp"

class TemperatureSimulator : public SensorSimulatorBase
{
public:
    TemperatureSimulator(std::string name);
    ~TemperatureSimulator() override = default;

    bool initialize() override;
    void reset() override;
    float read() override;

protected:
    void generateNextSample() override;

private:
    float m_current_temperature;
};