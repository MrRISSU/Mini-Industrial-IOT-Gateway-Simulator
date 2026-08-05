// To-Do: Configure using Nano Protocol Buffer (nanopb)
// include this in main.cpp

#include "DeviceSimulator.hpp"

DeviceSimulConf_t EnergyMeterConf =
{
    .dscr = "Energy Meter",
    .enable = 1,
    .interval = 5,
    .blockCount = 2,

    .block =
    {
        {
            .dscr = "Line - 1",
            .enable = 1,
            .tagCount = 4,
            .tag =
            {
                {.name = "Volt_L1",      .dataType = "f32"},
                {.name = "Current_L1",   .dataType = "f32"},
                {.name = "Frequency_L1", .dataType = "f32"},
                {.name = "Power_L1",     .dataType = "f32"},
            }
        },
        {
            .dscr = "Line - 2",
            .enable = 1,
            .tagCount = 4,
            .tag =
            {
                {.name = "Volt_L2",      .dataType = "f32"},
                {.name = "Current_L2",   .dataType = "f32"},
                {.name = "Frequency_L2", .dataType = "f32"},
                {.name = "Power_L2",     .dataType = "f32"},
            }
        },
    }
};

DeviceSimulConf_t DHT20Conf =
{
    .dscr = "DHT20",
    .enable = 1,
    .interval = 5,
    .blockCount = 1,

    .block =
    {
        {
            .dscr = "Line - 1",
            .enable = 1,
            .tagCount = 2,
            .tag =
            {
                {.name = "Temperature",  .dataType = "f32"},
                {.name = "Humidity",     .dataType = "f32"}
            }
        }
    }
};

