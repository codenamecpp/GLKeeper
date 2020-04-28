#pragma once

#include "GuiDefs.h"

// parse json attribute and get size or position value along with units in which it specified
// for example: ["10%", 200] - 10 percents and 200 pixels
bool GuiParsePixelsOrPercents(cxx::json_document_node node, eGuiUnits& output_units, int& output_value);

// parse gui anchors state from json node
// example: ["left", "right"] - left and right anchors are set
// example: ["all"] - all anchors are set 
bool GuiParseAnchors(cxx::json_node_array node, GuiAnchors& output_anchors);

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

class GuiVisibilityConditions
{
public:
    // readonly
    GuiWidget* mRootWidget = nullptr;
    
public:
    GuiVisibilityConditions() = default;

    void SetState(cxx::unique_string stateIdentifier, bool isTrue);
    void Invalidate();
    void SetVisibility();
    void Clear();

    void Bind(GuiWidget* widget);

private:
    void SetVisibility(GuiWidget* widget);
    bool IsConditionTrue(const std::)

private:

    struct ConditionState
    {
        bool mIsTrue = false;
        bool mIsInitialized = false;
    };

    // combination of enabled or disabled conditions -
    // each condition is name identifier
    std::map<cxx::unique_string, ConditionState> mConditionsMap;

    bool mInvalidated = false;
};