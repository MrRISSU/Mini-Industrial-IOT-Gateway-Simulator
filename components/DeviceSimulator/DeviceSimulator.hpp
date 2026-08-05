/******************************************************************************
 * @file DeviceSimulator.hpp
 * @brief Header for Device Simulator capable of simulating multiple tags
 *
 * Author : Huwairis Ibnu Kabeer
 * License: MIT
 ******************************************************************************/

#ifndef DEVICE_SIMULATOR_HPP
#define DEVICE_SIMULATOR_HPP

/*==========================================================================*/
/* Includes                                                                 */
/*==========================================================================*/

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "TagsRegister.hpp"

/*==========================================================================*/
/* Macros                                                                   */
/*==========================================================================*/

#define MAX_TAGS 10    // Maximum tags per block 
#define MAX_BLOCKS 5   // Maximum blocks per device 

/*==========================================================================*/
/* Structures                                                               */
/*==========================================================================*/

typedef struct DeviceSimulTag
{
    char name[20];
    char dataType[5];
} DeviceSimulTag_t;

typedef struct DeviceSimulBlock
{
    char dscr[20];
    int enable;
    int tagCount;
    DeviceSimulTag_t tag[MAX_TAGS];
} DeviceSimulBlock_t;

typedef struct
{
    char dscr[20];
    int enable;
    int interval;
    int blockCount;
    DeviceSimulBlock_t block[MAX_BLOCKS];
} DeviceSimulConf_t;

/*==========================================================================*/
/* Classes                                                                  */
/*==========================================================================*/

class DeviceSimulator
{
public:
    DeviceSimulator();
    ~DeviceSimulator();

    // Initializes the simulator and spawns the task on Core 1
    bool Initialise(DeviceSimulConf_t conf, TagsRegistry& registry);
    
    // Safely halts the simulation task
    void Destroy();

private:
    DeviceSimulConf_t deviceSimulConf;
    
    TaskHandle_t m_task_handle;
    bool m_is_running;

    // Pointer to access the global or shared registry
    TagsRegistry* pTagsRegistry;

    // Static FreeRTOS bridge to member functions 
    static void taskWorker(void* pvParameters);
    
    // Iterates through blocks and tags to generate mock data
    void processSimulation();
    
    // Standard helper for injecting pseudo-random noise variations 
    float generateGaussianNoise(float mean, float stddev);
};

#endif