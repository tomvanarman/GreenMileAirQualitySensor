#pragma once

#include <ArduinoJson.h>

class JsonBuilder
{
public:
    // Builds a JSON string using a lambda to populate fields.
    static String BuildJson(std::function<void(JsonDocument &)> builder, bool pretty = false)
    {
        JsonDocument doc;
        builder(doc);
        String output;
        if (pretty)
        {
            serializeJsonPretty(doc, output);
        }
        else
        {
            serializeJson(doc, output);
        }
        return output;
    }
};