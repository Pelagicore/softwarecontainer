struct testData {
    const char *title;
    const char *data;
};

void PrintTo(const testData& d, ::std::ostream* os) {
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
    }



};
