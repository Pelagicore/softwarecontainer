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

#pragma once
#include "baseconfigstore.h"

namespace softwarecontainer {

class FilteredConfigStore : public BaseConfigStore
{
    LOG_DECLARE_CLASS_CONTEXT("FCS", "FilteredConfigStore");
public:
    FilteredConfigStore(std::unique_ptr<ServiceManifestLoader> loader);

    /**
     * @brief Returns all capability IDs
     *
     * @return a vector of all IDs as strings
     */
    const std::vector<std::string> IDs() const;

    /**
     * @brief Returns all Gateway configurations for certain Capabilities.
     *
     * @param capIDs vector of strings representating Capability IDs
     * @return GatewayConfiguration containing config mapped by the given Capability ids.
     */
    GatewayConfiguration configsByID(const std::vector<std::string> &capIDs) const;

    /**
     * @brief Returns all Gateway configurations for a certain Capability.
     *
     * @param capID a string representation of the Capability ID
     * @return GatewayConfiguration containing config mapped by the given ID.
     */
    GatewayConfiguration configByID(const std::string &capID) const;

};

} // namespace softwarecontainer
