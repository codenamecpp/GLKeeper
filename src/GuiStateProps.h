#pragma once

#include "GuiDefs.h"

// properties setter for certain widget state which is user defined
class GuiStateProps: public cxx::noncopyable
{
public:
    // readonly
    std::string mStateName;

public:
    GuiStateProps();
    virtual ~GuiStateProps();

    // deserialize state properties from json node
    // @returns false on error
    virtual bool LoadStateProperties(cxx::json_node_object jsonNode);

    // apply state properties to specific widget
    // @param widget: Affected widget
    virtual void SetupStateProperties(GuiWidget* widget);

    // construct state clone
    virtual GuiStateProps* Clone();

protected:
    // copy properties
    GuiStateProps(GuiStateProps* copyWidget);

    // base widget properties

    GuiAnchors mAnchorsProp;

    eGuiUnits mOriginUnitsXProp = eGuiUnits_Pixels;
    eGuiUnits mOriginUnitsYProp = eGuiUnits_Pixels;
    Point mOriginProp;

    eGuiUnits mPositionUnitsXProp = eGuiUnits_Pixels;
    eGuiUnits mPositionUnitsYProp = eGuiUnits_Pixels;
    Point mPositionProp;

    eGuiUnits mSizeUnitsWProp = eGuiUnits_Pixels;
    eGuiUnits mSizeUnitsHProp = eGuiUnits_Pixels;
    Point mSizeProp;

    bool mEnabledProp = true;
    bool mVisibleProp = true;
    bool mClipChildrenProp = false;

    bool mAnchorsSet: 1;
    bool mOriginSet: 1;
    bool mPositionSet: 1;
    bool mSizeSet: 1;
    bool mEnabledSet: 1;
    bool mVisibleSet: 1;
    bool mClipChildrenSet: 1;
};