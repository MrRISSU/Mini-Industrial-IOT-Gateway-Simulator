/******************************************************************************
 * @file MQTTClient.cpp
 * @brief MQTT 5.0 Client implementation.
 *
 * Author : Huwairis Ibnu Kabeer
 * License: MIT
 ******************************************************************************/

/*==========================================================================*/
/* Includes                                                                 */
/*==========================================================================*/

#include <cstring>
#include "MQTTClient.hpp"

/*==========================================================================*/
/* Namespace                                                                */
/*==========================================================================*/

namespace mqtt
{

/*==========================================================================*/
/* Public Member Functions                                                  */
/*==========================================================================*/

Client::Client()
    : mpTransport_(nullptr)
    , mpConfig_(nullptr)
    , state_(ConnectionState::Disconnected)
    , nextPacketId_(1)
    , rxBufferLen_(0)
{
}

Client::~Client()
{
    Destroy();
}

bool Client::Initialise(Transport& transport, Config& config)
{
    mpTransport_ = &transport;
    mpConfig_ = &config;
    state_ = ConnectionState::Disconnected;
    rxBufferLen_ = 0;
    return true;
}

bool Client::Destroy()
{
    if (IsConnected())
    {
        Disconnect();
    }

    mpTransport_ = nullptr;
    mpConfig_ = nullptr;
    state_ = ConnectionState::Disconnected;
    return true;
}

bool Client::Connect()
{
    if (mpTransport_ == nullptr || mpConfig_ == nullptr)
    {
        return false;
    }

    if (!mpConfig_->isEnabled)
    {
        return false;
    }

    state_ = ConnectionState::Connecting;

    if (!mpTransport_->Connect(mpConfig_->brokerUri, mpConfig_->port, mpConfig_->timeoutMs))
    {
        state_ = ConnectionState::Disconnected;
        return false;
    }

    // Build MQTT 5.0 CONNECT Packet
    Packet connectPacket;
    connectPacket.type = PacketType::CONNECT;
    connectPacket.connectData.isCleanStart = mpConfig_->isCleanStart;
    connectPacket.connectData.keepAliveSec = mpConfig_->keepAliveSec;

    // Client ID
    std::size_t cLen = mpConfig_->clientIdLen;
    if (cLen >= kMaxClientIdLength)
    {
        cLen = kMaxClientIdLength - 1;
    }
    for (std::size_t i = 0; i < cLen; ++i)
    {
        connectPacket.connectData.clientId[i] = mpConfig_->clientId[i];
    }
    connectPacket.connectData.clientIdLen = cLen;

    // Credentials
    connectPacket.connectData.usernameLen = mpConfig_->usernameLen;
    if (mpConfig_->usernameLen > 0)
    {
        std::size_t uLen = (mpConfig_->usernameLen < kMaxUsernameLength) ? mpConfig_->usernameLen : (kMaxUsernameLength - 1);
        for (std::size_t i = 0; i < uLen; ++i)
        {
            connectPacket.connectData.username[i] = mpConfig_->username[i];
        }
        connectPacket.connectData.usernameLen = uLen;
    }

    connectPacket.connectData.passwordLen = mpConfig_->passwordLen;
    if (mpConfig_->passwordLen > 0)
    {
        std::size_t pLen = (mpConfig_->passwordLen < kMaxPasswordLength) ? mpConfig_->passwordLen : (kMaxPasswordLength - 1);
        for (std::size_t i = 0; i < pLen; ++i)
        {
            connectPacket.connectData.password[i] = mpConfig_->password[i];
        }
        connectPacket.connectData.passwordLen = pLen;
    }

    // Will Settings
    connectPacket.connectData.hasWill = mpConfig_->hasWill;
    if (mpConfig_->hasWill)
    {
        connectPacket.connectData.willQos = mpConfig_->willQos;
        connectPacket.connectData.isWillRetain = mpConfig_->isWillRetain;

        std::size_t wtLen = (mpConfig_->willTopicLen < kMaxWillTopicLength) ? mpConfig_->willTopicLen : (kMaxWillTopicLength - 1);
        for (std::size_t i = 0; i < wtLen; ++i)
        {
            connectPacket.connectData.willTopic[i] = mpConfig_->willTopic[i];
        }
        connectPacket.connectData.willTopicLen = wtLen;

        std::size_t wpLen = (mpConfig_->willPayloadLen < kMaxWillPayloadSize) ? mpConfig_->willPayloadLen : kMaxWillPayloadSize;
        for (std::size_t i = 0; i < wpLen; ++i)
        {
            connectPacket.connectData.willPayload[i] = mpConfig_->willPayload[i];
        }
        connectPacket.connectData.willPayloadLen = wpLen;
    }

    // Encode CONNECT Packet
    if (!encoder_.Encode(connectPacket))
    {
        mpTransport_->Disconnect();
        state_ = ConnectionState::Disconnected;
        return false;
    }

    // Write packet to Transport
    if (!mpTransport_->Write(encoder_.GetBuffer(), encoder_.GetEncodedLength(), mpConfig_->timeoutMs))
    {
        mpTransport_->Disconnect();
        state_ = ConnectionState::Disconnected;
        return false;
    }

    // Read CONNACK Response
    int bytesRead = mpTransport_->Read(rxBuffer_.data(), kMaxPacketSize, mpConfig_->timeoutMs);
    if (bytesRead <= 0)
    {
        mpTransport_->Disconnect();
        state_ = ConnectionState::Disconnected;
        return false;
    }
    std::size_t readBytes = static_cast<std::size_t>(bytesRead);

    Packet responsePacket;
    if (!decoder_.Decode(rxBuffer_.data(), readBytes, responsePacket))
    {
        mpTransport_->Disconnect();
        state_ = ConnectionState::Disconnected;
        return false;
    }

    if (responsePacket.type == PacketType::CONNACK && responsePacket.connackData.reasonCode == ReasonCode::Success)
    {
        state_ = ConnectionState::Connected;
        return true;
    }

    mpTransport_->Disconnect();
    state_ = ConnectionState::Disconnected;
    return false;
}

bool Client::Disconnect()
{
    if (mpTransport_ == nullptr)
    {
        return false;
    }

    if (IsConnected())
    {
        Packet disconnectPacket;
        disconnectPacket.type = PacketType::DISCONNECT;
        disconnectPacket.reason_code = ReasonCode::NormalDisconnection;

        if (encoder_.Encode(disconnectPacket))
        {
            mpTransport_->Write(encoder_.GetBuffer(), encoder_.GetEncodedLength(), mpConfig_->timeoutMs);
        }
    }

    mpTransport_->Disconnect();
    state_ = ConnectionState::Disconnected;
    return true;
}

bool Client::Publish(const char* topic,
                    const std::uint8_t* payload,
                    std::size_t payloadLen,
                    Qos qos,
                    bool retain)
{
    if (!IsConnected() || topic == nullptr)
    {
        return false;
    }

    etl::string_view topicView(topic);
    if (!Topic::IsValidTopicName(topicView))
    {
        return false;
    }

    Qos actualQos = ResolveQos(qos);

    Packet pubPacket;
    pubPacket.type = PacketType::PUBLISH;
    pubPacket.qos = actualQos;
    pubPacket.retain = retain;
    pubPacket.dup = false;

    if (actualQos != Qos::QoS0)
    {
        pubPacket.packet_id = GetNextPacketId();
    }

    std::size_t tLen = topicView.length();
    if (tLen >= kMaxTopicLength)
    {
        tLen = kMaxTopicLength - 1;
    }
    for (std::size_t i = 0; i < tLen; ++i)
    {
        pubPacket.topic[i] = topic[i];
    }
    pubPacket.topic[tLen] = '\0';
    pubPacket.topic_length = tLen;

    std::size_t pLen = (payloadLen < kMaxPayloadSize) ? payloadLen : kMaxPayloadSize;
    if (payload != nullptr && pLen > 0)
    {
        for (std::size_t i = 0; i < pLen; ++i)
        {
            pubPacket.payload[i] = payload[i];
        }
    }
    pubPacket.payload_length = pLen;

    if (!encoder_.Encode(pubPacket))
    {
        return false;
    }

    return mpTransport_->Write(encoder_.GetBuffer(), encoder_.GetEncodedLength(), mpConfig_->timeoutMs);
}

bool Client::Subscribe(const char* topicFilter, Qos qos)
{
    if (!IsConnected() || topicFilter == nullptr)
    {
        return false;
    }

    etl::string_view filterView(topicFilter);
    if (!Topic::IsValidTopicFilter(filterView))
    {
        return false;
    }

    Qos actualQos = ResolveQos(qos);

    Packet subPacket;
    subPacket.type = PacketType::SUBSCRIBE;
    subPacket.packet_id = GetNextPacketId();
    subPacket.subscribeData.packetId = subPacket.packet_id;
    subPacket.subscribeData.filterCount = 1;

    std::size_t fLen = filterView.length();
    if (fLen >= kMaxTopicLength)
    {
        fLen = kMaxTopicLength - 1;
    }
    for (std::size_t i = 0; i < fLen; ++i)
    {
        subPacket.subscribeData.filters[0].topicFilter[i] = topicFilter[i];
    }
    subPacket.subscribeData.filters[0].topicFilter[fLen] = '\0';
    subPacket.subscribeData.filters[0].topicFilterLen = fLen;
    subPacket.subscribeData.filters[0].maxQos = actualQos;

    if (!encoder_.Encode(subPacket))
    {
        return false;
    }

    if (!mpTransport_->Write(encoder_.GetBuffer(), encoder_.GetEncodedLength(), mpConfig_->timeoutMs))
    {
        return false;
    }

    // Read SUBACK Response
    int bytesRead = mpTransport_->Read(rxBuffer_.data(), kMaxPacketSize, mpConfig_->timeoutMs);
    if (bytesRead <= 0)
    {
        return false;
    }
    std::size_t readBytes = static_cast<std::size_t>(bytesRead);

    Packet responsePacket;
    if (!decoder_.Decode(rxBuffer_.data(), readBytes, responsePacket))
    {
        return false;
    }

    if (responsePacket.type == PacketType::SUBACK && responsePacket.subackData.packetId == subPacket.packet_id)
    {
        return (responsePacket.subackData.reasonCodeCount > 0 && 
                static_cast<std::uint8_t>(responsePacket.subackData.reasonCodes[0]) < 0x80);
    }

    return false;
}

bool Client::Unsubscribe(const char* topicFilter)
{
    if (!IsConnected() || topicFilter == nullptr)
    {
        return false;
    }

    Packet unsubPacket;
    unsubPacket.type = PacketType::UNSUBSCRIBE;
    unsubPacket.packet_id = GetNextPacketId();

    std::size_t fLen = std::strlen(topicFilter);
    if (fLen >= kMaxTopicLength)
    {
        fLen = kMaxTopicLength - 1;
    }
    for (std::size_t i = 0; i < fLen; ++i)
    {
        unsubPacket.topic[i] = topicFilter[i];
    }
    unsubPacket.topic[fLen] = '\0';
    unsubPacket.topic_length = fLen;

    if (!encoder_.Encode(unsubPacket))
    {
        return false;
    }

    return mpTransport_->Write(encoder_.GetBuffer(), encoder_.GetEncodedLength(), mpConfig_->timeoutMs);
}

bool Client::Ping()
{
    if (!IsConnected())
    {
        return false;
    }

    Packet pingPacket;
    pingPacket.type = PacketType::PINGREQ;

    if (!encoder_.Encode(pingPacket))
    {
        return false;
    }

    return mpTransport_->Write(encoder_.GetBuffer(), encoder_.GetEncodedLength(), mpConfig_->timeoutMs);
}

bool Client::Loop(int timeoutMs)
{
    if (!IsConnected())
    {
        return false;
    }

    int bytesRead = mpTransport_->Read(rxBuffer_.data(), kMaxPacketSize, timeoutMs);
    if (bytesRead <= 0)
    {
        // No data read or timeout
        return true;
    }

    std::size_t readBytes = static_cast<std::size_t>(bytesRead);

    Packet rxPacket;
    if (!decoder_.Decode(rxBuffer_.data(), readBytes, rxPacket))
    {
        return false;
    }

    if (rxPacket.type == PacketType::PUBLISH)
    {
        if (messageCallback_.is_valid())
        {
            messageCallback_(rxPacket.topic.data(), rxPacket.payload.data(), rxPacket.payload_length);
        }

        // Send PUBACK if QoS1
        if (rxPacket.qos == Qos::QoS1)
        {
            Packet pubackPacket;
            pubackPacket.type = PacketType::PUBACK;
            pubackPacket.packet_id = rxPacket.packet_id;
            pubackPacket.reason_code = ReasonCode::Success;
            if (encoder_.Encode(pubackPacket))
            {
                mpTransport_->Write(encoder_.GetBuffer(), encoder_.GetEncodedLength(), mpConfig_->timeoutMs);
            }
        }
    }

    return true;
}

void Client::SetMessageCallback(MessageCallback cb)
{
    messageCallback_ = cb;
}

ConnectionState Client::GetState() const
{
    return state_;
}

bool Client::IsConnected() const
{
    return (state_ == ConnectionState::Connected);
}

/*==========================================================================*/
/* Private Member Functions                                                 */
/*==========================================================================*/

std::uint16_t Client::GetNextPacketId()
{
    std::uint16_t id = nextPacketId_++;
    if (nextPacketId_ == 0)
    {
        nextPacketId_ = 1;
    }
    return id;
}

Qos Client::ResolveQos(Qos qos) const
{
    if (qos == Qos::Default)
    {
        return (mpConfig_ != nullptr) ? mpConfig_->defaultQos : Qos::QoS0;
    }
    return qos;
}

} // namespace mqtt
