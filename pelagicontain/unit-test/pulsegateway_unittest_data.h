/*
 *   Copyright (C) 2014 Pelagicore AB
 *   All rights reserved.
 */


struct pulseTestData {
    const char *title;
    const char *data;
    const std::string audio;
};

/* void PrintTo(const pulseTestData& d, ::std::ostream* os) { */
/*     *os << "Title: " << d.title; */
/*     *os << "\nData: " << d.data; */
/* } */

const struct pulseTestData invalidConfigs[] = {
    {
        "Missing closing }",
        "{\"audio\":  \"true\"",
        "true"
    },
    {
        "Missing value",
        "{\"audio\": }",
        "true"
    },
    {
        "Missing key",
        "{\"\": \"true\"}",
        "true"
    },
    {
        "Bad key",
        "{\"audo\": }",
        "true"
    }
};

const struct pulseTestData validConfigs[] = {
    {
        "Minimal config",
        "{"
            "\"audio\": \"true\""
        "}",
        "true"
    },
    {
        "Confusing config",
        "{"
            "\"audo\": \"true\","
            "\"audio\": \"true\""
        "}",
        "true"
    }
};

const struct pulseTestData disablingConfigs[] = {
    {
        "Minimal config",
        "{"
            "\"audio\": \"false\""
        "}",
        "false"
    },
    {
        "Confusing config",
        "{"
            "\"audo\": \"true\","
            "\"audio\": \"false\""
        "}",
        "false"
    },
    {
        "Incorrect value",
        "{\"audio\":  \"faulty-value\" }",
        "false"
    }
};
