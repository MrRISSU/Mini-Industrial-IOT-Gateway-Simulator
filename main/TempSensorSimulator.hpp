#pragma once
#include "SensorSimulatorBase.hpp"

class TemperatureSimulator : public SensorSimulatorBase
{
public:
    TemperatureSimulator(std::string name);
    ~TemperatureSimulator() override = default;

    bool initialize() override;
    void reset() override;

protected:
    void generateNextSample() override;

private:
    float m_current_temperature;
};