
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


#include <vector>

struct testData
{
    const char *title;
    const char *data;
    const std::vector<std::string> names;
    const std::vector<std::string> majors;
    const std::vector<std::string> minors;
    const std::vector<std::string> modes;
};

void PrintTo(const testData &d, ::std::ostream *os)
{
    *os << "Title: " << d.title;
    *os << "\nData: " << d.data;
}

const struct testData invalidConfigs[] = {
    {
        "Missing closing ]",
        "{\"devices\": ["
        "                  {"
        "                      \"name\":  \"tty0\","
        "                      \"major\": \"4\","
        "                      \"minor\": \"0\","
        "                      \"mode\":  \"666\""
        "                  }"
        "}"
    },
    {
        "Bad top level key",
        "{\"wrongKey\": ["
        "                  {"
        "                      \"name\":  \"tty0\","
        "                      \"major\": \"4\","
        "                      \"minor\": \"0\","
        "                      \"mode\":  \"666\""
        "                  }"
        "               ]"
        "}"
    },
    {
        "Missing name",
        "{\"devices\": ["
        "                  {"
        "                      \"major\": \"4\","
        "                      \"minor\": \"0\","
        "                      \"mode\":  \"666\""
        "                  }"
        "               ]"
        "}"
    },
    {
        "Missing major and name and minor",
        "{\"devices\": ["
        "                  {"
        "                      \"mode\":  \"666\""
        "                  }"
        "               ]"
        "}"
    },
    {
        "Missing all props",
        "{\"devices\": ["
        "                  {"
        "                  }"
        "               ]"
        "}"
    },
    {
        "'Devices' is a string?!",
        "{\"devices\": \"hej\" }"
    },
    {
        "Last device malformed",
        "{\"devices\": ["
        "                  {"
        "                      \"name\":  \"tty0\","
        "                      \"major\": \"4\","
        "                      \"minor\": \"0\","
        "                      \"mode\":  \"666\""
        "                  },"
        "                  {"
        "                      \"major\": \"4\","
        "                      \"minor\": \"0\","
        "                      \"mode\":  \"666\""
        "                  }"
        "              ]"
        "}"
    },
};

const struct testData validConfigs[] = {
    {
        "Correct 1",
        "{\"devices\": ["
        "                  {"
        "                      \"name\":  \"tty0\","
        "                      \"major\": \"4\","
        "                      \"minor\": \"0\","
        "                      \"mode\":  \"666\""
        "                  }"
        "              ]"
        "}",
        {"tty0"},
        {"4"},
        {"0"},
        {"666"}
    },
    {
        "Correct 2",
        "{\"devices\": [ ]}",
        {},
        {},
        {},
        {}
    },
    {
        "Correct 2",
        "{\"devices\": ["
        "                  {"
        "                      \"name\":  \"tty0\","
        "                      \"major\": \"4\","
        "                      \"minor\": \"0\","
        "                      \"mode\":  \"666\""
        "                  },"
        "                  {"
        "                      \"name\":  \"tty1\","
        "                      \"major\": \"4\","
        "                      \"minor\": \"0\","
        "                      \"mode\":  \"400\""
        "                  },"
        "                  {"
        "                      \"name\":  \"/dev/galcore\","
        "                      \"major\": \"199\","
        "                      \"minor\": \"0\","
        "                      \"mode\":  \"666\""
        "                  }"
        "              ]"
        "}",
        {"tty0", "tty1", "/dev/galcore"},
        {"4", "4", "199"},
        {"0", "0", "0"},
        {"666", "400", "666"}
    },
};
