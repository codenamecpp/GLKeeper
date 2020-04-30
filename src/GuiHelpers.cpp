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

bool GuiParseColor(cxx::json_document_node node, Color32& output_color)
{
    output_color = 0; // set to black
    if (cxx::json_node_array arrayNode = node)
    {
        if (cxx::json_get_array_item(arrayNode, 0, output_color.mR) &&
            cxx::json_get_array_item(arrayNode, 1, output_color.mG) &&
            cxx::json_get_array_item(arrayNode, 2, output_color.mB) &&
            cxx::json_get_array_item(arrayNode, 3, output_color.mA))
        {
            return true;
        }
        debug_assert(false);
    }
    if (cxx::json_node_string stringNode = node)
    {
        std::string hexString = stringNode.get_value();

        char* p;
        unsigned int color_value = strtoul(hexString.c_str(), &p, 16);
        if (*p == 0)
        {
            output_color = color_value;
            // reverse channels order
            std::reverse(output_color.mChannels, output_color.mChannels + 4);
            return true;
        }
        debug_assert(false);
    }
    return false;
}

GuiWidget* GuiGetChildWidgetByPath(GuiWidget* parent_widget, const std::string& child_path)
{
    if (parent_widget == nullptr)
    {
        debug_assert(false);
        return nullptr;
    }

    if (!cxx::contains(child_path, '/'))
    {
        return parent_widget->GetChild(child_path);
    }
    
    std::string currentName;
    cxx::string_tokenizer tokenizer(child_path);

    GuiWidget* currentWidget = parent_widget;
    for (;;)
    {
        if (!tokenizer.get_next(currentName))
            break;

        debug_assert(!currentName.empty());
        if (GuiWidget* child = currentWidget->GetChild(currentName))
        {
            currentWidget = child;
            continue;
        }
        // not found
        currentWidget = nullptr;
        break;
    }

    return currentWidget;
}
