/* ****************************************************************************
* Author    : Huwairis Ibnu Kabeer
* Company   : MrRISSU
* email     : huwairisibnukabeer777@gmail.com
* Mob No    : +91-9447504259
* File name : TagsRegister.hpp
* ****************************************************************************/

#include <type_traits>
#include <new>
#include <stdint.h>
#include <sys/types.h>
#include <time.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define SIZE_TAG_NAME  20

typedef enum __attribute__((packed)) DataType
{
    TYPE_EMPTY,
    TYPE_I8,   TYPE_UI8,
    TYPE_I16,  TYPE_UI16,
    TYPE_I32,  TYPE_UI32, TYPE_F32,
    TYPE_I64,  TYPE_UI64, TYPE_F64
} dataType_t;

typedef enum __attribute__((packed)) Endian
{
    BE, // BIG_ENDIAN
    LE, // LITTLE_ENDIAN
    BEBS, // BIG_ENDIAN_BYTE_SWAP
    LEBS // LITTLE_ENDIAN_BYTE_SWAP
} endian_t;

typedef struct Value
{
    // Tracks what is currently stored
    dataType_t type;
    // Anonymous union makes syntax cleaner (no extra dot needed)
    union {
        int8_t   i8;   uint8_t  ui8;
        int16_t  i16;  uint16_t ui16;
        int32_t  i32;  uint32_t ui32; float f32;
        int64_t  i64;  uint64_t ui64; double f64;
    };
} value_t;


typedef struct Tag
{
    char name[SIZE_TAG_NAME];
    value_t value;
    endian_t endian;
    // track change in value
    // if delta is zero, it means there is no change
    int delta;
    // Time of value change
    time_t time;
}tag_t;


class TagsRegistry
{
private:
    SemaphoreHandle_t tagsRegistryMutex;
    tag_t* tagRegister;
    size_t tagRegisterSize;

    int FindRegister(const char* name);
    int FindFreeRegister();
    int GetRegisterIndex(const char* name);
protected:
public:
    TagsRegistry();
    ~TagsRegistry();
    int Initialize(size_t tagsMax);
    template <typename type>
    bool Write(const char* name, type value);
    template <typename type>
    type Read(const char* name);
};


