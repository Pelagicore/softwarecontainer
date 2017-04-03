/*
 * Copyright (C) 2017 Pelagicore AB
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

#include "softwarecontainer-common.h"
#include "containerabstractinterface.h"
#include "jsonparser.h"


namespace softwarecontainer {

class Device
{
    LOG_DECLARE_CLASS_CONTEXT("DN", "Device Node");

public:
    Device(std::string name = "", int mode = -1);
    /*
     * @brief Copy constructor
     */
    Device(const Device &dev);

    /**
     * @brief Configures this device by parsing the supplied JSON configuration string
     *
     * @param element A JSON configuration item.
     * @returns false if an error was encountered while parsing, true otherwise.
     */
    bool parse(const json_t *element);

    /**
     * @brief Activates this device by running mknod and chmod commands which are run in the
     * container.
     *
     * @return true upon success, false otherwise
     */
    bool activate(std::shared_ptr<ContainerAbstractInterface> container);

    /*
     * @brief Calculate the device node mode after applying white-listing policy
     * i.e. if the appliedMode is 706 and storedMode is 622 then the calculated mode will be 726
     *
     * @return integer value representing new device mode
     */
    void calculateDeviceMode(const int appliedMode);

    /* needed getters */
    const std::string getName();
    int getMode();
    bool getIsconfigured ();

    /* needed setters*/
    void setMode(int _mode);

private:
    std::string m_name;
    /*
     * @brief Holds device mode which consist 3 integer number between 0 and 7 which represents
     * permissions for owner, groups and all users
     * */
    int m_mode;

    /*
    * @brief Holds the flag that whether the device is configured or not
    */
    bool m_isConfigured;
};

} // namespace softwarecontainer
