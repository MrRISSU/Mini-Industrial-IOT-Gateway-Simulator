/******************************************************************************
 * @file Transport.hpp
 * @brief Abstract interface for byte-stream communication channels.
 *
 * Author : Huwairis Ibnu Kabeer
 * License: MIT
 ******************************************************************************/

#pragma once

/*==========================================================================*/
/* Includes                                                                 */
/*==========================================================================*/

#include <cstdint>
#include <cstddef>

/*==========================================================================*/
/* Classes                                                                  */
/*==========================================================================*/

class Transport
{
public:
    virtual ~Transport() = default;
    virtual bool Connect(const char* host, uint16_t port, int timeout_ms) = 0;
    virtual int Read(uint8_t* buffer, std::size_t len, int timeout_ms) = 0;
    virtual int Write(const uint8_t* buffer, std::size_t len, int timeout_ms) = 0;
    virtual void Disconnect() = 0;
};
