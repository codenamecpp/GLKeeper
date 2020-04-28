#include "pch.h"
#include "GuiHelpers.h"
#include "GuiWidget.h"
#include "TexturesManager.h"
#include "GuiPictureBox.h"

bool GuiParsePixelsOrPercents(cxx::json_document_node node, eGuiUnits& output_units, int& output_value)
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

bool GuiParseAnchors(cxx::json_node_array node, GuiAnchors& output_anchors)
{
    if (node)
    {
        output_anchors.mB = false;
        output_anchors.mL = false;
        output_anchors.mR = false;
        output_anchors.mT = false;

        std::string string_value;
        for (cxx::json_node_string currElement = node.first_child(); currElement;
            currElement = currElement.next_sibling())
        {
            string_value = currElement.get_value();
            if (string_value == "left")
            {
                output_anchors.mL = true;
                continue;
            }
            if (string_value == "right")
            {
                output_anchors.mR = true;
                continue;
            }
            if (string_value == "top")
            {
                output_anchors.mT = true;
                continue;
            }
            if (string_value == "bottom")
            {
                output_anchors.mL = true;
                continue;
            }
            if (string_value == "all")
            {
                output_anchors.mL = true;
                output_anchors.mR = true;
                output_anchors.mT = true;
                output_anchors.mB = true;
                return true;
            }
            debug_assert(false);
        }
        return true;
    }
    return false;
}
