#pragma once

#include "GuiDefs.h"

// parse json attribute and get size or position value along with units in which it specified
// for example: ["10%", 200] - 10 percents and 200 pixels
bool GuiParsePixelsOrPercents(cxx::json_document_node node, eGuiUnits& output_units, int& output_value);

// parse gui anchors state from json node
// example: ["left", "right"] - left and right anchors are set
// example: ["all"] - all anchors are set 
bool GuiParseAnchors(cxx::json_node_array node, GuiAnchors& output_anchors);

// parse color from json attribute
// as array of ints example: [0, 255, 0, 255] - rgba
// as string example: "00ff00ff" - rgba
bool GuiParseColor(cxx::json_document_node node, Color32& output_color);

// try to cast base widget pointer to specific widget class
template<typename TargetWidgetClass>
inline TargetWidgetClass* GuiCastWidgetClass(GuiWidget* sourceWidget)
{
    TargetWidgetClass* resultWidget = nullptr;
    debug_assert(sourceWidget);
    if (sourceWidget)
    {
        GuiWidgetMetaClass* target_metaclass = &TargetWidgetClass::MetaClass;
        for (GuiWidgetMetaClass* currMetaclass = sourceWidget->mMetaClass; currMetaclass; 
            currMetaclass = currMetaclass->mParentClass)
        {
            if (target_metaclass == currMetaclass)
            {
                resultWidget = (TargetWidgetClass*) sourceWidget;
                break;
            }
        }
    }
    return resultWidget;
}

template<typename TFunc>
inline void GuiRefreshVisibility(GuiWidget* sourceWidget, TFunc func)
{
    debug_assert(sourceWidget);
    if (sourceWidget->mVisibilityConditions.non_null())
    {
        bool isVisible = sourceWidget->mVisibilityConditions.evaluate_expression(func);
        sourceWidget->SetVisible(isVisible);
    }
}

template<typename TFunc>
inline void GuiRefreshVisibilityRecursive(GuiWidget* sourceWidget, TFunc func)
{
    debug_assert(sourceWidget);
    GuiRefreshVisibility(sourceWidget, func);
    // process children
    for (GuiWidget* currChild = sourceWidget->GetChild(); currChild;
        currChild = currChild->NextSibling())
    {
        GuiRefreshVisibilityRecursive(currChild, func);
    }
}