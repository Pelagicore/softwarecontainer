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

#include <unistd.h>
#include "periodictemperature.h"
#include "temperatureinterface.h"

/*
 *  Sets temperature periodically
 *  Until it reaches 40 and starts over from 0 after that
 */
void setTemperaturePeriodically(TemperatureInterface *interface)
{
    double temperature = 0.0;
    while(true) {

        if(temperature > 40) {
            temperature = 0;
        }

        temperature = temperature + 0.5;
        interface->setTemperature(temperature);

        // wait two seconds before setting temperature again
        sleep(2);
    }
}

