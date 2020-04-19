#pragma once

#include "GuiDefs.h"

// parse json attribute and get size or position value along with units in which it specified
// for example: ["10%", 200] - 10 percents and 200 pixels
inline bool GetPixelsOrPercentsValue(const cxx::json_document_node& node, eGuiUnits& output_units, int& output_value)
{
    // percents or pixels
    if (cxx::json_node_string string_node = node)
    {
        std::string string_value = string_node.get_value();
        if (cxx::ends_with(string_value, '%'))
        {
            // looks like percents
            if (::sscanf(string_value.c_str(), "%d", &output_value) > 0)
            {
                output_units = eGuiUnits_Percents;
                return true;
            }
        }
        else
        {
            if (::sscanf(string_value.c_str(), "%d", &output_value) > 0)
            {
                output_units = eGuiUnits_Pixels;
                return true;
            }
        }
        return false;
    }

    // pixels
    if (cxx::json_node_numeric numeric_node = node)
    {
        output_units = eGuiUnits_Pixels;
        output_value = numeric_node.get_value_integer();
        return true;
    }
    return false;
}