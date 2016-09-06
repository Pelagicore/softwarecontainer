#ifndef JSONPARSER_H
#define JSONPARSER_H

#include <ivi-logging.h>
#include <jansson.h>

namespace softwarecontainer {
    class JSONParser {
    LOG_DECLARE_CLASS_CONTEXT("JSON", "JSONParser");

    protected:
        bool read(const json_t *element, const char *key, std::string &result)
        {
            json_t *value = json_object_get(element, key);
            if (!value) {
                return false;
            }

            if (!json_is_string(value)) {
                log_error() << "json element is not a string";
                return false;
            }

            result = json_string_value(value);
            return true;
        }

        bool read(const json_t *element, const char *key, bool &result)
        {
            json_t *value = json_object_get(element, key);
            if (!value) {
                return false;
            }

            if (!json_is_boolean(value)) {
                log_error() << "json element is not a boolean";
                return false;
            }

            result = json_is_true(value);
            return true;
        }
    };
}
#endif // JSONPARSER_H
