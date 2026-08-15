/******************************************************************************
 * @file MQTTConfig.cpp
 * @brief Global default configuration instance definition for MQTT 5.0.
 *
 * Author : Huwairis Ibnu Kabeer
 * License: MIT
 ******************************************************************************/

/*==========================================================================*/
/* Includes                                                                 */
/*==========================================================================*/

#include "MQTTConfig.hpp"

/*==========================================================================*/
/* Namespace                                                                */
/*==========================================================================*/

namespace mqtt
{

/*==========================================================================*/
/* Configuration                                                            */
/*==========================================================================*/

Config DefaultConf = 
{
    .description     = "ESP32_MQTT_Node",
    .isEnabled       = true,
    .brokerUri       = "mqtt://192.168.1.100",
    .port            = 1883,
    .keepAliveSec    = 60,
    .timeoutMs       = 5000,
    .rxTaskPriority  = 5,
    .txTaskPriority  = 4,
    .isCleanStart    = true,

    .clientId        = {'E', 'S', 'P', '3', '2', '_', 'G', 'a', 't', 'e', 'w', 'a', 'y'},
    .clientIdLen     = 13,

    .username        = {},
    .usernameLen     = 0,

    .password        = {},
    .passwordLen     = 0,

    .defaultQos      = Qos::QoS0,

    .hasWill         = false,
    .willTopic       = {},
    .willTopicLen    = 0,

    .willPayload     = {},
    .willPayloadLen  = 0,

    .willQos         = Qos::QoS0,
    .isWillRetain    = false
};

} // namespace mqtt