#pragma once

#include "GuiDefs.h"

// parse json attribute and get size or position value along with units in which it specified
// example: ["10%", 200] - 10 percents and 200 pixels
bool GuiParsePixelsOrPercents(cxx::json_document_node node, eGuiUnits& output_units, int& output_value);

// parse gui anchors state from json node
// example: ["left", "right"] - left and right anchors are set
// example: "all" - all anchors are set 
bool GuiParseAnchors(cxx::json_document_node node, GuiAnchors& output_anchors);

// parse color from json attribute
// as array of ints example: [0, 255, 0, 255] - rgba
// as string example: "00ff00ff" - rgba
bool GuiParseColor(cxx::json_document_node node, Color32& output_color);

// get child widget by path
// example: "first/second/third"
GuiWidget* GuiGetChildWidgetByPath(GuiWidget* parent_widget, const std::string& child_path);