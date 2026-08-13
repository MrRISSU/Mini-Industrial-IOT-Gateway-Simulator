/******************************************************************************
 * @file TagsRegister.hpp
 * @brief A thread-safe, centralized registry for dynamic storage and retrieval of IoT tag values, deltas, and timestamps.
 *
 * Author : Huwairis Ibnu Kabeer
 * License: MIT
 ******************************************************************************/

/*==========================================================================*/
/* Includes                                                                 */
/*==========================================================================*/

#include "TagsRegister.hpp"
#include "esp_log.h"

/*==========================================================================*/
/* Macros                                                                   */
/*==========================================================================*/

/*==========================================================================*/
/* Static Variables                                                         */
/*==========================================================================*/

static const char* TAG = "[TagRegistry]";

/*==========================================================================*/
/* Static Configuration / Lookup Tables                                     */
/*==========================================================================*/

/*==========================================================================*/
/* Static Helper Functions                                                  */
/*==========================================================================*/

/*==========================================================================*/
/* Public Member Functions                                                  */
/*==========================================================================*/

TagsRegistry::TagsRegistry() : tagsRegistryMutex(nullptr)
{
}

TagsRegistry::~TagsRegistry() 
{
    if (tagsRegistryMutex)
        vSemaphoreDelete(tagsRegistryMutex);
    delete tagRegister;
    tagRegister = nullptr;
}

int TagsRegistry :: Initialize(size_t tagsMax)
{
    ESP_LOGD(TAG, "allocate memory for tagRegister (Size: %u)", tagsMax);
    tagRegister = new (std::nothrow) tag_t[tagsMax]{};
    if (tagRegister == nullptr)
    {
        ESP_LOGE(TAG, "Failed to allocate memory for tagRegister (Size: %u)", tagsMax);
        return -1;
    }
    tagRegisterSize = tagsMax;
    // Initialize mutex
    tagsRegistryMutex = xSemaphoreCreateMutex();
    if (tagsRegistryMutex == nullptr)
    {
        ESP_LOGE(TAG, "Failed to create Tag mutex");
        return -2;
    }
    return 0;
}

template <typename T> 
bool TagsRegistry::Write(const char* name, T value)
{
    if (xSemaphoreTake(tagsRegistryMutex, portMAX_DELAY) == pdTRUE)
    {
        int index = GetRegisterIndex(name);
        if (index == -1)
        {
            xSemaphoreGive(tagsRegistryMutex);
            return false;
        }

        // Safely store the name
        strlcpy(tagRegister[index].name, name, sizeof(tagRegister[index].name));
        
        // 1. Calculate Delta (only if the previous type matches the new type)
        // 2. Set the correct enum and union value based on the incoming type T
        if constexpr (std::is_same_v<T, int8_t>)
        {
            if (tagRegister[index].value.type == TYPE_I8) 
                tagRegister[index].delta = static_cast<int>(value - tagRegister[index].value.i8);
            tagRegister[index].value.type = TYPE_I8;
            tagRegister[index].value.i8 = value;
        } 
        else if constexpr (std::is_same_v<T, uint8_t>)
        {
            if (tagRegister[index].value.type == TYPE_UI8) 
                tagRegister[index].delta = static_cast<int>(value - tagRegister[index].value.ui8);
            tagRegister[index].value.type = TYPE_UI8;
            tagRegister[index].value.ui8 = value;
        }
        else if constexpr (std::is_same_v<T, int16_t>)
        {
            if (tagRegister[index].value.type == TYPE_I16) 
                tagRegister[index].delta = static_cast<int>(value - tagRegister[index].value.i16);
            tagRegister[index].value.type = TYPE_I16;
            tagRegister[index].value.i16 = value;
        }
        else if constexpr (std::is_same_v<T, uint16_t>)
        {
            if (tagRegister[index].value.type == TYPE_UI16) 
                tagRegister[index].delta = static_cast<int>(value - tagRegister[index].value.ui16);
            tagRegister[index].value.type = TYPE_UI16;
            tagRegister[index].value.ui16 = value;
        }
        else if constexpr (std::is_same_v<T, int32_t>)
        {
            if (tagRegister[index].value.type == TYPE_I32) 
                tagRegister[index].delta = static_cast<int>(value - tagRegister[index].value.i32);
            tagRegister[index].value.type = TYPE_I32;
            tagRegister[index].value.i32 = value;
        }
        else if constexpr (std::is_same_v<T, uint32_t>)
        {
            if (tagRegister[index].value.type == TYPE_UI32) 
                tagRegister[index].delta = static_cast<int>(value - tagRegister[index].value.ui32);
            tagRegister[index].value.type = TYPE_UI32;
            tagRegister[index].value.ui32 = value;
        }
        else if constexpr (std::is_same_v<T, float>)
        {
            if (tagRegister[index].value.type == TYPE_F32) 
                tagRegister[index].delta = static_cast<int>(value - tagRegister[index].value.f32);
            tagRegister[index].value.type = TYPE_F32;
            tagRegister[index].value.f32 = value;
        }
        else if constexpr (std::is_same_v<T, double>)
        {
            if (tagRegister[index].value.type == TYPE_F64) 
                tagRegister[index].delta = static_cast<int>(value - tagRegister[index].value.f64);
            tagRegister[index].value.type = TYPE_F64;
            tagRegister[index].value.f64 = value;
        }
        else
        {
            ESP_LOGE(TAG, "Unsupported type passed to Write()");
            xSemaphoreGive(tagsRegistryMutex);
            return false;
        }

        // Update the timestamp using standard ESP-IDF / POSIX time
        time_t now;
        time(&now);
        tagRegister[index].time = now;
        
        xSemaphoreGive(tagsRegistryMutex);
    }
    return true;
}

template bool TagsRegistry::Write<int8_t>(const char*, int8_t);
template bool TagsRegistry::Write<uint8_t>(const char*, uint8_t);
template bool TagsRegistry::Write<int16_t>(const char*, int16_t);
template bool TagsRegistry::Write<uint16_t>(const char*, uint16_t);
template bool TagsRegistry::Write<int32_t>(const char*, int32_t);
template bool TagsRegistry::Write<uint32_t>(const char*, uint32_t);
template bool TagsRegistry::Write<float>(const char*, float);
template bool TagsRegistry::Write<double>(const char*, double);

template <typename T> 
T TagsRegistry::Read(const char* name)
{
    // Zero-initialize the return value to ensure deterministic behavior on failure
    T returnValue = T{}; 

    if (xSemaphoreTake(tagsRegistryMutex, portMAX_DELAY) == pdTRUE)
    {
        if (name == nullptr)
        {
            ESP_LOGE(TAG, "Empty Tag Name provided to Read()");
            xSemaphoreGive(tagsRegistryMutex);
            return returnValue;
        }

        int index = FindRegister(name);
        if (index < 0)
        {
            // Use warning level for missing tags, as this is recoverable
            ESP_LOGW(TAG, "Tag not found: %s", name);
            xSemaphoreGive(tagsRegistryMutex);
            return returnValue;
        }

        // Type matching and extraction
        if constexpr (std::is_same_v<T, int8_t>)
        {
            if (tagRegister[index].value.type == TYPE_I8)
            {
                returnValue = tagRegister[index].value.i8;
            }
            else
            {
                ESP_LOGW(TAG, "Type mismatch on Read for tag: %s", name);
            }
        }
        else if constexpr (std::is_same_v<T, uint8_t>)
        {
            if (tagRegister[index].value.type == TYPE_UI8)
            {
                returnValue = tagRegister[index].value.ui8;
            }
            else
            {
                ESP_LOGW(TAG, "Type mismatch on Read for tag: %s", name);
            }
        }
        else if constexpr (std::is_same_v<T, int16_t>)
        {
            if (tagRegister[index].value.type == TYPE_I16)
            {
                returnValue = tagRegister[index].value.i16;
            }
            else
            {
                ESP_LOGW(TAG, "Type mismatch on Read for tag: %s", name);
            }
        }
        else if constexpr (std::is_same_v<T, uint16_t>)
        {
            if (tagRegister[index].value.type == TYPE_UI16)
            {
                returnValue = tagRegister[index].value.ui16;
            }
            else
            {
                ESP_LOGW(TAG, "Type mismatch on Read for tag: %s", name);
            }
        }
        else if constexpr (std::is_same_v<T, int32_t>)
        {
            if (tagRegister[index].value.type == TYPE_I32)
            {
                returnValue = tagRegister[index].value.i32;
            }
            else
            {
                ESP_LOGW(TAG, "Type mismatch on Read for tag: %s", name);
            }
        }
        else if constexpr (std::is_same_v<T, uint32_t>)
        {
            if (tagRegister[index].value.type == TYPE_UI32)
            {
                returnValue = tagRegister[index].value.ui32;
            }
            else
            {
                ESP_LOGW(TAG, "Type mismatch on Read for tag: %s", name);
            }
        }
        else if constexpr (std::is_same_v<T, float>)
        {
            if (tagRegister[index].value.type == TYPE_F32)
            {
                returnValue = tagRegister[index].value.f32;
            }
            else
            {
                ESP_LOGW(TAG, "Type mismatch on Read for tag: %s", name);
            }
        }
        else if constexpr (std::is_same_v<T, double>)
        {
            if (tagRegister[index].value.type == TYPE_F64)
            {
                returnValue = tagRegister[index].value.f64;
            }
            else
            {
                ESP_LOGW(TAG, "Type mismatch on Read for tag: %s", name);
            }
        }
        else
        {
            // Failsafe for unhandled types
            ESP_LOGE(TAG, "Unsupported type requested for tag: %s", name);
        }

        // Ensure mutex is given back on the success/matching path
        xSemaphoreGive(tagsRegistryMutex);
    }
    else
    {
        ESP_LOGE(TAG, "Failed to take mutex in Read()");
    }
    
    return returnValue;
}

template int8_t TagsRegistry::Read<int8_t>(const char*);
template uint8_t TagsRegistry::Read<uint8_t>(const char*);
template int16_t TagsRegistry::Read<int16_t>(const char*);
template uint16_t TagsRegistry::Read<uint16_t>(const char*);
template int32_t TagsRegistry::Read<int32_t>(const char*);
template uint32_t TagsRegistry::Read<uint32_t>(const char*);
template float TagsRegistry::Read<float>(const char*);
template double TagsRegistry::Read<double>(const char*);

bool TagsRegistry::Delete(const char* name)
{
    if (xSemaphoreTake(tagsRegistryMutex, portMAX_DELAY) == pdTRUE)
    {
        int index = FindRegister(name);
        if (index < 0)
        {
            ESP_LOGW(TAG, "Tag not found for deletion: %s", name);
            xSemaphoreGive(tagsRegistryMutex);
            return false;
        }

        // Clear the tag entry
        memset(&tagRegister[index], 0, sizeof(tag_t));
        xSemaphoreGive(tagsRegistryMutex);
        return true;
    }
    else
    {
        ESP_LOGE(TAG, "Failed to take mutex in Delete()");
        return false;
    }
}

/*==========================================================================*/
/* Private Member Functions                                                 */
/*==========================================================================*/

int TagsRegistry :: FindRegister(const char* name)
{
    for (int index = 0; index < tagRegisterSize; index++)
    {
        if (strcmp(tagRegister[index].name, name) == 0)
        {
            return index;
        }
    }
    return -1;
}

int TagsRegistry :: FindFreeRegister()
{
    for (int index = 0; index < tagRegisterSize; index++)
    {
        if (tagRegister[index].name[0] == '\0')
        {
            return index;
        }
    }
    return -1;
}

int TagsRegistry :: GetRegisterIndex(const char* name)
{
    if (tagRegister == nullptr || name == nullptr)
    {
        return -1;
    }

    int index = FindRegister(name);
    if (index == -1)
    {
        index = FindFreeRegister();
        if (index == -1)
        {
            ESP_LOGE(TAG, "No Free Tag Register\r\n");
            return -1;
        }
    }
    return index;
}
