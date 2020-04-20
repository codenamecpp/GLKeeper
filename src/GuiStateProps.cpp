#include "pch.h"
#include "GuiStateProps.h"
#include "GuiHelpers.h"
#include "GuiWidget.h"

GuiStateProps::GuiStateProps()
    : mAnchorsSet()
    , mOriginSet()
    , mPositionSet()
    , mSizeSet()
    , mEnabledSet()
    , mVisibleSet()
    , mClipChildrenSet()
{
}

GuiStateProps::GuiStateProps(GuiStateProps* copyWidget)
    : mAnchorsProp(copyWidget->mAnchorsProp)
    , mOriginUnitsXProp(copyWidget->mOriginUnitsXProp)
    , mOriginUnitsYProp(copyWidget->mOriginUnitsYProp)
    , mOriginProp(copyWidget->mOriginProp)
    , mPositionUnitsXProp(copyWidget->mPositionUnitsXProp)
    , mPositionUnitsYProp(copyWidget->mPositionUnitsYProp)
    , mPositionProp(copyWidget->mPositionProp)
    , mSizeUnitsWProp(copyWidget->mSizeUnitsWProp)
    , mSizeUnitsHProp(copyWidget->mSizeUnitsHProp)
    , mSizeProp(copyWidget->mSizeProp)
    , mEnabledProp(copyWidget->mEnabledProp)
    , mVisibleProp(copyWidget->mVisibleProp)
    , mClipChildrenProp(copyWidget->mClipChildrenProp)
    , mAnchorsSet(copyWidget->mAnchorsSet)
    , mOriginSet(copyWidget->mOriginSet)
    , mPositionSet(copyWidget->mPositionSet)
    , mSizeSet(copyWidget->mSizeSet)
    , mEnabledSet(copyWidget->mEnabledSet)
    , mVisibleSet(copyWidget->mVisibleSet)
    , mClipChildrenSet(copyWidget->mClipChildrenSet)
{
}

GuiStateProps::~GuiStateProps()
{
}

bool GuiStateProps::LoadStateProperties(cxx::json_node_object documentNode)
{
    if (!documentNode)
    {
        debug_assert(false);
        return false;
    }

    mStateName = documentNode.get_element_name();

    mVisibleSet = cxx::json_get_attribute(documentNode, "visible", mVisibleProp);
    mEnabledSet = cxx::json_get_attribute(documentNode, "enabled", mEnabledProp);
    mClipChildrenSet = cxx::json_get_attribute(documentNode, "clip_children", mClipChildrenProp);

    // origin
    if (cxx::json_node_array prop_position = documentNode["origin"])
    {
        mOriginSet = GetPixelsOrPercentsValue(prop_position[0], mOriginUnitsXProp, mOriginProp.x) &&
            GetPixelsOrPercentsValue(prop_position[1], mOriginUnitsYProp, mOriginProp.y);

        debug_assert(mOriginSet);
    }

    // position
    if (cxx::json_node_array prop_position = documentNode["pos"])
    {
        mPositionSet = GetPixelsOrPercentsValue(prop_position[0], mPositionUnitsXProp, mPositionProp.x) &&
            GetPixelsOrPercentsValue(prop_position[1], mPositionUnitsYProp, mPositionProp.y);

        debug_assert(mPositionSet);
    }
    // size
    if (cxx::json_node_array prop_size = documentNode["size"])
    {
        mSizeSet = GetPixelsOrPercentsValue(prop_size[0], mSizeUnitsWProp, mSizeProp.x) &&
            GetPixelsOrPercentsValue(prop_size[1], mSizeUnitsHProp, mSizeProp.y);

        debug_assert(mSizeSet);
    }
    // anchors
    if (cxx::json_node_object prop_anchors = documentNode["anchors"])
    {
        cxx::json_get_attribute(prop_anchors, "left", mAnchorsProp.mL);
        cxx::json_get_attribute(prop_anchors, "right", mAnchorsProp.mR);
        cxx::json_get_attribute(prop_anchors, "top", mAnchorsProp.mT);
        cxx::json_get_attribute(prop_anchors, "bottom", mAnchorsProp.mB);
        mAnchorsSet = true;
    }
    return true;
}

void GuiStateProps::SetupStateProperties(GuiWidget* widget)
{
    if (widget == nullptr)
    {
        debug_assert(false);
        return;
    }
    if (mEnabledSet)
    {
        widget->SetEnabled(mEnabledProp);
    }

    if (mVisibleSet)
    {
        widget->SetVisible(mVisibleProp);
    }

    if (mClipChildrenSet)
    {
        widget->SetClipChildren(mClipChildrenProp);
    }

    if (mOriginSet)
    {
        widget->SetOrigin(mOriginProp, mOriginUnitsXProp, mOriginUnitsYProp);
    }

    if (mPositionSet)
    {
        widget->SetPosition(mPositionProp, mPositionUnitsXProp, mPositionUnitsYProp);
    }

    if (mSizeSet)
    {
        widget->SetSize(mSizeProp, mSizeUnitsWProp, mSizeUnitsHProp);
    }
    
    if (mAnchorsSet)
    {
        widget->SetAnchors(mAnchorsProp);
    }
}

GuiStateProps* GuiStateProps::Clone()
{
    GuiStateProps* propClone = new GuiStateProps(this);
    return propClone;
}
