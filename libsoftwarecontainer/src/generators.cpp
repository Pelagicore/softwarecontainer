
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


#include <sys/time.h>
#include "ifaddrs.h"

#include "generators.h"

/*
 * Increase the counter and return an IP number based on that.
 */
std::string Generator::gen_ip_addr(const char *ip_addr_net)
{

    counter++;
    if (counter < 2 || counter > 254) {
        counter = 2;
    }

    char ip[20];
    snprintf(ip, sizeof(ip), "%s%d", ip_addr_net, counter);
    return std::string(ip);
}

std::string Generator::gen_ct_name()
{
    static const char alphanum[] = "abcdefghijklmnopqrstuvwxyz";
    struct timeval time;
    char name[10];

    gettimeofday(&time, nullptr);
    srand((time.tv_sec * 1000) + (time.tv_usec / 1000));

    for (size_t i = 0; i < sizeof(name); i++) {
        name[i] = alphanum[rand() % (sizeof(alphanum) - 1)];
    }

    name[sizeof(name) - 1] = '\0';
    return std::string(name);
}
