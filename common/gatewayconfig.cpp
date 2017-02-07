/*
 * Copyright (C) 2016-2017 Pelagicore AB
 *
 * Permission to use, copy, modify, and/or distribute this software for
 * any purpose with or without fee is hereby granted, provided that the
 * above copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL
 * WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR
 * BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES
 * OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
 * WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
 * ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
 * SOFTWARE.
 *
 * For further information see LICENSE
 */

#include "gatewayconfig.h"

namespace softwarecontainer {

GatewayConfiguration::GatewayConfiguration()
{
}

GatewayConfiguration::GatewayConfiguration(const GatewayConfiguration &gwConf)
{
    if (isError(append(gwConf))) {
        throw SoftwareContainerError("Failed to create GatewayConfiguration");
    }
}

GatewayConfiguration::~GatewayConfiguration()
{
    for (auto &it : m_configMap) {
        json_decref(it.second);
    }
}

ReturnCode GatewayConfiguration::append(const std::string &id, const std::string &jsonConf)
{
    json_error_t jsonError;
    size_t flags = 0; // default flags

    json_t *json = json_loads(jsonConf.c_str(), flags, &jsonError);
    if (nullptr == json) {
        log_error() << "Could not parse given string to JSON. Due to: " << jsonError.text << " at line: " << jsonError.line;
        return ReturnCode::FAILURE;
    }

    auto ret = append(id, json);
    json_decref(json);
    return ret;
}

ReturnCode GatewayConfiguration::append(const std::string &id, json_t *sourceArray)
{
    auto search = m_configMap.find(id);
    if (search != m_configMap.end()) {
        // Get the arrays
        json_t *destArray = search->second;
        json_t *backupArray = json_deep_copy(destArray);

        // Add all elements of the source array to the destination array
        size_t index;
        json_t *value;
        json_array_foreach(sourceArray, index, value) {
            json_t *copy = json_deep_copy(value);
            if (json_array_append(destArray, copy) < 0) {
                log_error() << "Could not add Gateway Config to json array: " << id;
                m_configMap[id] = backupArray;
                json_decref(destArray);
                return ReturnCode::FAILURE;
            }
        }
        json_decref(backupArray);
    } else {
        m_configMap[id] = sourceArray;
        json_incref(sourceArray);
    }

    return ReturnCode::SUCCESS;
}

ReturnCode GatewayConfiguration::append(const GatewayConfiguration &source)
{
    for (auto &conf : source.m_configMap) {
        if (isError(append(conf.first, conf.second))) {
            return ReturnCode::FAILURE;
        }
    }
    return ReturnCode::SUCCESS;
}

json_t *GatewayConfiguration::config(const std::string &gatewayId) const
{
    if (m_configMap.count(gatewayId) == 0) {
         return nullptr;
    }

    return json_deep_copy(m_configMap.at(gatewayId));
}

bool GatewayConfiguration::empty()
{
    return m_configMap.empty();
}

std::vector<std::string> GatewayConfiguration::ids() const
{
    std::vector<std::string> ids;
    for (auto &pair : m_configMap) {
        ids.push_back(pair.first);
    }

    return ids;
}

} // namespace softwarecontainer
