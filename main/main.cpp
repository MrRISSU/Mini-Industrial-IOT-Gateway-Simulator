#include "TempSensorSimulator.hpp"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

extern "C" void app_main() {
    TemperatureSimulator engineTemp("Engine_Temp_1");
    
    if (engineTemp.initialize()) {
        // Run at 1000ms intervals with priority 5
        engineTemp.startSimulation(1000, 5); 
    }

    // Main task can now go to sleep or do other things
    while (true) {
        vTaskDelay(pdMS_TO_TICKS(10000));
    }
}