/******************************************************************************
 * @file MQTTTopic.hpp
 * @brief MQTT Topic validation, wildcard matching, and level extraction.
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
#include <etl/string_view.h>

#include "MQTTConfig.hpp"

/*==========================================================================*/
/* Namespace                                                                */
/*==========================================================================*/

namespace mqtt
{

/*==========================================================================*/
/* Classes                                                                  */
/*==========================================================================*/

/**
 * @brief Utility class for zero-allocation MQTT topic operations.
 */
class Topic
{
public:
    Topic() = delete;
    ~Topic() = delete;

    /**
     * @brief Performs zero-allocation wildcard matching between a filter and topic.
     * @param topicFilter Subscription topic filter (may contain '+' or '#').
     * @param actualTopic Incoming published topic name (must not contain wildcards).
     * @return true if actualTopic matches topicFilter according to MQTT rules.
     */
    static bool Match(etl::string_view topicFilter, etl::string_view actualTopic);

    /**
     * @brief Validates a published topic name.
     * @param topic Topic string to validate.
     * @return true if valid (non-empty, <= kMaxTopicLength, no wildcards).
     */
    static bool IsValidTopicName(etl::string_view topic);

    /**
     * @brief Validates a subscription topic filter.
     * @param topicFilter Filter string to validate.
     * @return true if valid (non-empty, <= kMaxTopicLength, correct wildcard placement).
     */
    static bool IsValidTopicFilter(etl::string_view topicFilter);

    /**
     * @brief Counts the number of '/' delimited levels in a topic string.
     * @param topic Topic string to analyze.
     * @return Count of topic levels.
     */
    static std::size_t GetLevelCount(etl::string_view topic);

    /**
     * @brief Extracts a specific topic level into a static buffer.
     * @param topic Source topic string.
     * @param levelIndex 0-indexed level position to extract.
     * @param outBuffer Destination character array buffer.
     * @param maxLen Capacity of outBuffer.
     * @param outLen Output byte length of extracted level string.
     * @return true if level extracted successfully, false if index out of bounds or buffer too small.
     */
    static bool ExtractLevel(etl::string_view topic,
                             std::size_t levelIndex,
                             char* outBuffer,
                             std::size_t maxLen,
                             std::size_t& outLen);
};

} // namespace mqtt
