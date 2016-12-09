
/*
 * Copyright (C) 2016 Pelagicore AB
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

#include "devicenodelogic.h"

int DeviceNodeLogic::calculateDeviceMode(const int storedMode, const int appliedMode)
{
    int mode = storedMode;

    if (storedMode != appliedMode) {

        ((appliedMode/100) >= (storedMode/100)) ?
                mode = (appliedMode/100) * 100 : mode = (storedMode/100) * 100;

        (((appliedMode/10)%10) >= ((storedMode/10)%10)) ?
                mode += (((appliedMode/10)%10) * 10) : mode += (((storedMode/10)%10) * 10);

        ((appliedMode%10) >= (storedMode%10)) ?
                mode += (appliedMode%10) :  mode += (storedMode%10);

    }

    return mode;
}

