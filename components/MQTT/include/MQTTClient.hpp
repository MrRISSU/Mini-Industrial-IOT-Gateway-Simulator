
/******************************************************************************
 * @file MQTTClient.hpp
 * @brief MQTT 5.0 Client class declaration.
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
#include <etl/delegate.h>

#include "Transport.hpp"
#include "MQTTConfig.hpp"
#include "MQTTDecoder.hpp"
#include "MQTTEncoder.hpp"
#include "MQTTPacket.hpp"
#include "MQTTTopic.hpp"

/*==========================================================================*/
/* Namespace                                                                */
/*==========================================================================*/

namespace mqtt
{

/*==========================================================================*/
/* Type Definitions / Aliases                                                */
/*==========================================================================*/

/**
 * @brief Delegate type for incoming PUBLISH message callbacks.
 */
using MessageCallback = etl::delegate<void(const char* topic, const std::uint8_t* payload, std::size_t payloadLen)>;

/*==========================================================================*/
/* Classes                                                                  */
/*==========================================================================*/

/**
 * @brief Main MQTT 5.0 Client managing network lifecycle, publishing,
 * subscribing, and incoming message handling over Transport.
 */
class Client
{
public:
    Client();
    ~Client();

    /**
     * @brief Initialises client with network transport and configuration.
     * @param transport Network stream abstraction instance.
     * @param config Reference to runtime configuration (defaults to DefaultConf).
     * @return true if initialised successfully.
     */
    bool Initialise(Transport& transport, Config& config = DefaultConf);

    /**
     * @brief Destroys client instance and closes connections.
     * @return true if destroyed cleanly.
     */
    bool Destroy();

    /**
     * @brief Initiates MQTT 5.0 connection handshake with broker.
     * @return true if connected successfully (CONNACK received with success).
     */
    bool Connect();

    /**
     * @brief Sends DISCONNECT packet and closes underlying network transport.
     * @return true if disconnected cleanly.
     */
    bool Disconnect();

    /**
     * @brief Publishes a message to a specific topic.
     * @param topic Destination topic string.
     * @param payload Raw message payload bytes.
     * @param payloadLen Length of payload in bytes.
     * @param qos Quality of Service level (defaults to Qos::Default, resolving to config.defaultQos).
     * @param retain Retain flag.
     * @return true if publish succeeded.
     */
    bool Publish(const char* topic,
                 const std::uint8_t* payload,
                 std::size_t payloadLen,
                 Qos qos = Qos::Default,
                 bool retain = false);

    /**
     * @brief Subscribes to a topic filter.
     * @param topicFilter Destination topic filter (may contain wildcards).
     * @param qos Quality of Service level (defaults to Qos::Default, resolving to config.defaultQos).
     * @return true if subscribe packet sent and SUBACK received successfully.
     */
    bool Subscribe(const char* topicFilter, Qos qos = Qos::Default);

    /**
     * @brief Unsubscribes from a topic filter.
     * @param topicFilter Filter string to unsubscribe from.
     * @return true if unsubscribe packet sent successfully.
     */
    bool Unsubscribe(const char* topicFilter);

    /**
     * @brief Transmits a PINGREQ packet to keep connection alive.
     * @return true if ping request transmitted successfully.
     */
    bool Ping();

    /**
     * @brief Network loop to read and process incoming stream bytes from transport.
     * @param timeoutMs Maximum read wait timeout in milliseconds.
     * @return true if loop executed cleanly without critical transport error.
     */
    bool Loop(int timeoutMs = 100);

    /**
     * @brief Registers callback for incoming PUBLISH messages.
     * @param cb Callback delegate instance.
     */
    void SetMessageCallback(MessageCallback cb);

    /**
     * @brief Gets current connection state.
     * @return Current ConnectionState enum.
     */
    ConnectionState GetState() const;

    /**
     * @brief Checks if client is currently connected.
     * @return true if connected.
     */
    bool IsConnected() const;

private:
    std::uint16_t GetNextPacketId();
    Qos ResolveQos(Qos qos) const;

    Transport* mpTransport_ = nullptr;
    Config* mpConfig_ = nullptr;
    Encoder encoder_;
    Decoder decoder_;
    ConnectionState state_ = ConnectionState::Disconnected;
    std::uint16_t nextPacketId_ = 1;
    MessageCallback messageCallback_;

    etl::array<std::uint8_t, kMaxPacketSize> rxBuffer_;
    std::size_t rxBufferLen_ = 0;
};

using MQTTClient = Client;

} // namespace mqtt

