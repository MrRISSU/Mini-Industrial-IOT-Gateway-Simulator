/******************************************************************************
 * @file MQTTTopic.cpp
 * @brief MQTT Topic validation, wildcard matching, and level extraction implementation.
 *
 * Author : Huwairis Ibnu Kabeer
 * License: MIT
 ******************************************************************************/

/*==========================================================================*/
/* Includes                                                                 */
/*==========================================================================*/

#include "MQTTTopic.hpp"

/*==========================================================================*/
/* Namespace                                                                */
/*==========================================================================*/

namespace mqtt
{

/*==========================================================================*/
/* Public Static Member Functions                                           */
/*==========================================================================*/

bool Topic::Match(etl::string_view topicFilter, etl::string_view actualTopic)
{
    if (!IsValidTopicFilter(topicFilter) || !IsValidTopicName(actualTopic))
    {
        return false;
    }

    std::size_t filterIdx = 0;
    std::size_t topicIdx = 0;
    std::size_t filterLen = topicFilter.length();
    std::size_t topicLen = actualTopic.length();

    while (filterIdx < filterLen && topicIdx < topicLen)
    {
        if (topicFilter[filterIdx] == '#')
        {
            // '#' matches all remaining levels
            return true;
        }

        if (topicFilter[filterIdx] == '+')
        {
            // '+' matches a single level up to next '/' delimiter or end of string
            while (topicIdx < topicLen && actualTopic[topicIdx] != '/')
            {
                topicIdx++;
            }
            filterIdx++;
        }
        else if (topicFilter[filterIdx] == actualTopic[topicIdx])
        {
            filterIdx++;
            topicIdx++;
        }
        else
        {
            return false;
        }
    }

    // Check trailing multi-level wildcard '#'
    if (filterIdx < filterLen && topicFilter[filterIdx] == '#')
    {
        return true;
    }

    // Must reach end of both strings simultaneously for exact match
    return (filterIdx == filterLen && topicIdx == topicLen);
}

bool Topic::IsValidTopicName(etl::string_view topic)
{
    if (topic.empty() || topic.length() > kMaxTopicLength)
    {
        return false;
    }

    for (char c : topic)
    {
        if (c == '+' || c == '#' || c == '\0')
        {
            return false;
        }
    }

    return true;
}

bool Topic::IsValidTopicFilter(etl::string_view topicFilter)
{
    if (topicFilter.empty() || topicFilter.length() > kMaxTopicLength)
    {
        return false;
    }

    std::size_t len = topicFilter.length();
    for (std::size_t i = 0; i < len; ++i)
    {
        char c = topicFilter[i];
        if (c == '\0')
        {
            return false;
        }

        if (c == '#')
        {
            // '#' must be the last character in the filter
            if (i != len - 1)
            {
                return false;
            }
            // '#' must be preceded by '/' unless it's the only character
            if (i > 0 && topicFilter[i - 1] != '/')
            {
                return false;
            }
        }

        if (c == '+')
        {
            // '+' must be preceded by '/' or start of string
            if (i > 0 && topicFilter[i - 1] != '/')
            {
                return false;
            }
            // '+' must be followed by '/' or end of string
            if (i + 1 < len && topicFilter[i + 1] != '/')
            {
                return false;
            }
        }
    }

    return true;
}

std::size_t Topic::GetLevelCount(etl::string_view topic)
{
    if (topic.empty())
    {
        return 0;
    }

    std::size_t count = 1;
    for (char c : topic)
    {
        if (c == '/')
        {
            count++;
        }
    }
    return count;
}

bool Topic::ExtractLevel(etl::string_view topic,
                         std::size_t levelIndex,
                         char* outBuffer,
                         std::size_t maxLen,
                         std::size_t& outLen)
{
    outLen = 0;
    if (outBuffer == nullptr || maxLen == 0 || topic.empty())
    {
        return false;
    }

    std::size_t currentLevel = 0;
    std::size_t levelStart = 0;
    std::size_t len = topic.length();

    for (std::size_t i = 0; i <= len; ++i)
    {
        if (i == len || topic[i] == '/')
        {
            if (currentLevel == levelIndex)
            {
                std::size_t levelLength = i - levelStart;
                if (levelLength >= maxLen)
                {
                    return false; // Buffer space safety
                }

                for (std::size_t j = 0; j < levelLength; ++j)
                {
                    outBuffer[j] = topic[levelStart + j];
                }
                outBuffer[levelLength] = '\0';
                outLen = levelLength;
                return true;
            }

            currentLevel++;
            levelStart = i + 1;
        }
    }

    return false; // levelIndex out of bounds
}

} // namespace mqtt
