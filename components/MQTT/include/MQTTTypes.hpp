/******************************************************************************
 * @file MQTTTypes.hpp
 * @brief
 *
 * Author : Huwairis Ibnu Kabeer
 * License: MIT
 ******************************************************************************/

#pragma once

/*==========================================================================*/
/* Includes                                                                 */
/*==========================================================================*/

#include <cstdint>

/*==========================================================================*/
/* Namespace                                                                */
/*==========================================================================*/

namespace mqtt {

/*==========================================================================*/
/* Enumerations                                                             */
/*==========================================================================*/

enum class MqttPacketType : std::uint8_t
{
    // --- Connection Handling ---
    CONNECT     = 1,  // Client to Server: Initiate connection with credentials and keep-alive settings.
    CONNACK     = 2,  // Server to Client: Connection acknowledgment, returns status/reason codes.

    // --- Message Publishing ---
    PUBLISH     = 3,  // Bidirectional: Transports the actual message payload to/from a topic.
    PUBACK      = 4,  // Bidirectional: QoS 1 acknowledgment; confirms message receipt.
    PUBREC      = 5,  // Bidirectional: QoS 2 assured delivery part 1; message received by receiver.
    PUBREL      = 6,  // Bidirectional: QoS 2 assured delivery part 2; release message for delivery.
    PUBCOMP     = 7,  // Bidirectional: QoS 2 assured delivery part 3; transaction handshake complete.

    // --- Topic Subscriptions ---
    SUBSCRIBE   = 8,  // Client to Server: Request to subscribe to one or more topic filters.
    SUBACK      = 9,  // Server to Client: Subscription acknowledgment, returns granted QoS levels.
    UNSUBSCRIBE = 10, // Client to Server: Request to unsubscribe from specific topic filters.
    UNSUBACK    = 11, // Server to Client: Unsubscription acknowledgment.

    // --- Heartbeats & Maintenance ---
    PINGREQ     = 12, // Client to Server: Keep-alive request to maintain connection activity.
    PINGRESP    = 13, // Server to Client: Keep-alive response confirming the broker is alive.

    // --- Disconnection & Security ---
    DISCONNECT  = 14, // Bidirectional: Graceful termination notice for the MQTT session.
    AUTH        = 15  // Bidirectional: Enhanced security handshake for authentication (MQTT 5.0).
};


enum class MqttQos : std::uint8_t
{
    QoS0 = 0, // AtMostOnce  - Fire and forget (no acknowledgement)
    QoS1 = 1, // AtLeastOnce - Acknowledged delivery (via PUBACK)
    QoS2 = 2  // ExactlyOnce - Assured delivery (via 4-step handshake)
};


enum class MqttConnectionState : std::uint8_t
{
    Disconnected = 0, // Session inactive; default initial state or after a clean exit.
    Connecting   = 1, // Socket is open, and CONNECT packet has been sent to the broker.
    Connected    = 2, // CONNACK received with success; session is active and healthy.
    Subscribing  = 3  // Connected, but currently waiting on critical startup SUBSCRIBE/SUBACK flows.
};


enum class MqttReasonCode : std::uint8_t {
    // --- Success Codes ( < 0x80 ) ---
    Success                 = 0x00, // Connection accepted / Publish acknowledged / Success.
    NormalDisconnection     = 0x00, // Clean disconnection initiated by client or server.
    DisconnectWithWillMessage=0x04, // Disconnect cleanly, but trigger the Will Message.
    GrantedQos0             = 0x00, // Subscription accepted with maximum QoS 0.
    GrantedQos1             = 0x01, // Subscription accepted with maximum QoS 1.
    GrantedQos2             = 0x02, // Subscription accepted with maximum QoS 2.
    NoMatchingSubscribers   = 0x10, // Message published, but no topics matched the filter.

    // --- Client-Side Internal Errors ---
    ClientNetworkError      = 0x70, // Local socket failure or dropped TCP connection.
    ClientTimeout           = 0x71, // Broker failed to respond within keep-alive window.

    // --- Protocol & Authority Errors ( >= 0x80 ) ---
    UnspecifiedError        = 0x80, // Error occurred, but the broker won't reveal the cause.
    MalformedPacket         = 0x81, // Received data does not conform to MQTT specification.
    ProtocolError           = 0x82, // Packet received out of order or invalid state rule broken.
    ImplementationSpecific  = 0x83, // Valid packet, but unsupported by this specific broker build.
    NotAuthorized           = 0x87, // Credentials failed or client lacks topic permissions.
    ServerBusy              = 0x89, // Broker is overloaded; connection rejected temporarily.
    BadAuthenticationMethod = 0x8C, // The AUTH method string is not recognized or allowed.
    TopicNameInvalid        = 0x90, // Topic string contains illegal characters or formats.
    PacketIdInUse           = 0x91, // Collision; the packet identifier is already active.
    PacketTooLarge          = 0x95, // Payload exceeds the broker's maximum allowed limit.
    QuotaExceeded           = 0x97, // Message rate, topic count, or memory limits hit.
    KeepAliveTimeout        = 0x8D, // Server closed connection due to inactivity.
    SessionTakenOver        = 0x8E  // New client connected using the same Client ID.
};

} // namespace mqtt
