#include "pch.h"
#include "GuiWidgetActions.h"
#include "GuiWidget.h"
#include "GuiHelpers.h"
#include "GuiEvent.h"
#include "GuiPictureBox.h"

GuiWidgetActionsManager gGuiWidgetActionsManager;

//////////////////////////////////////////////////////////////////////////

GuiWidgetAction::GuiWidgetAction(GuiWidget* parentWidget) 
    : mParentWidget(parentWidget)
{
    debug_assert(mParentWidget);
}

GuiWidgetAction::GuiWidgetAction(GuiWidget* parentWidget, const GuiWidgetAction* copyAction)
    : mParentWidget(parentWidget)
    , mConditions(copyAction->mConditions)
    , mTargetPath(copyAction->mTargetPath)
{
    debug_assert(mParentWidget);
}

void GuiWidgetAction::PerformAction()
{
    if (!EvaluateConditions())
        return;

    GuiWidget* target = mParentWidget;
    if (!mTargetPath.empty())
    {
        target = GuiGetChildWidgetByPath(mParentWidget, mTargetPath);
    }
    debug_assert(target);
    if (target)
    {
        HandlePerformAction(target);
    }
}

void GuiWidgetAction::ReleaseAction()
{
    delete this;
}

bool GuiWidgetAction::Deserialize(cxx::json_node_object actionNode)
{
    std::string conditions;
    if (cxx::json_get_attribute(actionNode, "conditions", conditions))
    {
        if (!mConditions.parse_expression(conditions))
        {
            debug_assert(false);
            return false;
        }
    }
    cxx::json_get_attribute(actionNode, "target", mTargetPath);
    return HandleDeserialize(actionNode);
}

GuiWidgetAction* GuiWidgetAction::CloneAction(GuiWidget* parentWidget)
{
    return HandleCloneAction(parentWidget);
}

bool GuiWidgetAction::EvaluateConditions() const
{
    bool isTrue = true;
    if (mConditions.non_null())
    {
        isTrue = mConditions.evaluate_expression([this](const std::string& name)
        {
            if (name == "pressed")
            {
                return mParentWidget->IsSelected();
            }
            if (name == "hovered")
            {
                return mParentWidget->IsHovered();
            }
            if (name == "enabled")
            {
                return mParentWidget->IsEnabled();
            }
            if (name == "visible")
            {
                return mParentWidget->IsVisible();
            }
            debug_assert(false);
            return false;
        });
    }
    return isTrue;
}

//////////////////////////////////////////////////////////////////////////

class GuiWidgetActionShow: public GuiWidgetAction
{
public:
    GuiWidgetActionShow(GuiWidget* parentWidget)
        : GuiWidgetAction(parentWidget)
    {
    }
    GuiWidgetActionShow(GuiWidget* parentWidget, const GuiWidgetActionShow* copyAction)
        : GuiWidgetAction(parentWidget, copyAction)
        , mIsShow(copyAction->mIsShow)
    {
    }
    void HandlePerformAction(GuiWidget* targetWidget) override
    {
        targetWidget->SetVisible(mIsShow);
    }
    bool HandleDeserialize(cxx::json_node_object actionNode) override
    {
        if (!cxx::json_get_attribute(actionNode, "value", mIsShow))
        {
            int bp = 0;
        }
        return true;
    }
    GuiWidgetActionShow* HandleCloneAction(GuiWidget* parentWidget) override
    {
        debug_assert(parentWidget);
        return new GuiWidgetActionShow(parentWidget, this);
    }
public:
    bool mIsShow = true;
};

//////////////////////////////////////////////////////////////////////////

class GuiWidgetActionSize: public GuiWidgetAction
{
public:
    GuiWidgetActionSize(GuiWidget* parentWidget)
        : GuiWidgetAction(parentWidget)
    {
    }
    GuiWidgetActionSize(GuiWidget* parentWidget, const GuiWidgetActionSize* copyAction)
        : GuiWidgetAction(parentWidget, copyAction)
        , mUnitsW(copyAction->mUnitsW)
        , mUnitsH(copyAction->mUnitsH)
        , mSize(copyAction->mSize)
    {
    }
    void HandlePerformAction(GuiWidget* targetWidget) override
    {
        targetWidget->SetSize(mSize, mUnitsW, mUnitsH);
    }
    bool HandleDeserialize(cxx::json_node_object actionNode) override
    {
        cxx::json_node_array sizeNode = actionNode["value"];
        if (!GuiParsePixelsOrPercents(sizeNode[0], mUnitsW, mSize.x) ||
            !GuiParsePixelsOrPercents(sizeNode[1], mUnitsH, mSize.y))
        {
            debug_assert(false);
            return false;
        }
        return true;
    }
    GuiWidgetActionSize* HandleCloneAction(GuiWidget* parentWidget) override
    {
        debug_assert(parentWidget);
        return new GuiWidgetActionSize(parentWidget, this);
    }
private:
    eGuiUnits mUnitsW;
    eGuiUnits mUnitsH;
    Point mSize;
};

//////////////////////////////////////////////////////////////////////////

class GuiWidgetActionSetTexture: public GuiWidgetAction
{
public:
    GuiWidgetActionSetTexture(GuiWidget* parentWidget)
        : GuiWidgetAction(parentWidget)
    {
    }
    GuiWidgetActionSetTexture(GuiWidget* parentWidget, const GuiWidgetActionSetTexture* copyAction)
        : GuiWidgetAction(parentWidget, copyAction)
        , mTextureName(copyAction->mTextureName)
    {
    }
    void HandlePerformAction(GuiWidget* targetWidget) override
    {
        GuiPictureBox* pictureBox = GuiCastWidgetClass<GuiPictureBox>(targetWidget);
        if (pictureBox)
        {
            if (mTextureName.empty())
            {
                // clear current texture
                pictureBox->SetTexture(nullptr);
            }
            else
            {
                pictureBox->SetTexture(mTextureName);
            }
        }
        debug_assert(pictureBox);
    }
    bool HandleDeserialize(cxx::json_node_object actionNode) override
    {
        cxx::json_get_attribute(actionNode, "value", mTextureName);
        return true;
    }
    GuiWidgetActionSetTexture* HandleCloneAction(GuiWidget* parentWidget) override
    {
        debug_assert(parentWidget);
        return new GuiWidgetActionSetTexture(parentWidget, this);
    }
private:
    std::string mTextureName;
};

//////////////////////////////////////////////////////////////////////////

class GuiWidgetActionEmitEvent: public GuiWidgetAction
{
public:
    GuiWidgetActionEmitEvent(GuiWidget* parentWidget)
        : GuiWidgetAction(parentWidget)
    {
    }
    GuiWidgetActionEmitEvent(GuiWidget* parentWidget, const GuiWidgetActionEmitEvent* copyAction)
        : GuiWidgetAction(parentWidget, copyAction)
        , mEventId(copyAction->mEventId)
    {
    }
    void HandlePerformAction(GuiWidget* targetWidget) override
    {
        if (mEventId.empty())
        {
            debug_assert(false);
            return;
        }
        targetWidget->mActionsHolder.EmitEvent(mEventId);
    }
    bool HandleDeserialize(cxx::json_node_object actionNode) override
    {
        cxx::json_get_attribute(actionNode, "value", mEventId);
        debug_assert(!mEventId.empty());
        return true;
    }
    GuiWidgetActionEmitEvent* HandleCloneAction(GuiWidget* parentWidget) override
    {
        debug_assert(parentWidget);
        return new GuiWidgetActionEmitEvent(parentWidget, this);
    }
private:
    cxx::unique_string mEventId;
};


//////////////////////////////////////////////////////////////////////////

GuiWidgetActionsHolder::GuiWidgetActionsHolder(GuiWidget* actionsParentWidget)
    : mParentWidget(actionsParentWidget)
{
    debug_assert(mParentWidget);
}

GuiWidgetActionsHolder::~GuiWidgetActionsHolder()
{
    ClearActions();
}

void GuiWidgetActionsHolder::AddAction(cxx::unique_string eventId, GuiWidgetAction* action)
{
    if (cxx::contains_if(mActionsList, [&eventId, action](const EventActionStruct& curr)
        {
            return (curr.mAction == action) && (curr.mEventId == eventId);
        }))
    {
        debug_assert(false);
        return;
    }

    EventActionStruct evActionStruct;
        evActionStruct.mEventId = eventId;
        evActionStruct.mAction = action;
    mActionsList.push_back(evActionStruct);
}

void GuiWidgetActionsHolder::ClearActions()
{
    for (const EventActionStruct& curr: mActionsList)
    {
        curr.mAction->ReleaseAction();
    }
    mActionsList.clear();
}

void GuiWidgetActionsHolder::EmitEvent(cxx::unique_string eventId)
{
    for (const EventActionStruct& curr: mActionsList)
    {
        if (curr.mEventId == eventId)
        {
            curr.mAction->PerformAction();
        }
    }
}

void GuiWidgetActionsHolder::CopyActions(const GuiWidgetActionsHolder& source)
{
    debug_assert(mParentWidget);

    for (const EventActionStruct& curr_element: source.mActionsList)
    {
        GuiWidgetAction* action = curr_element.mAction->CloneAction(mParentWidget);
        debug_assert(action);

        if (action)
        {
            AddAction(curr_element.mEventId, action);
        }
    }
}

//////////////////////////////////////////////////////////////////////////

void GuiWidgetActionsManager::DeserializeActions(cxx::json_node_object actionsNode, GuiWidgetActionsHolder& actionsList)
{    
    // iterate events
    for (cxx::json_node_array currNode = actionsNode.first_child(); currNode;
        currNode = currNode.next_sibling())
    {
        cxx::unique_string eventId = cxx::unique_string(currNode.get_element_name());

        // iterate actions
        for (cxx::json_node_object currActionNode = currNode.first_child(); currActionNode;
            currActionNode = currActionNode.next_sibling())
        {
            GuiWidgetAction* widgetAction = DeserializeAction(currActionNode, actionsList.mParentWidget);
            if (widgetAction == nullptr)
            {
                debug_assert(false);
                continue;
            }

            actionsList.AddAction(eventId, widgetAction);
        }
    }    
}

GuiWidgetAction* GuiWidgetActionsManager::DeserializeAction(cxx::json_node_object actionNode, GuiWidget* actionsParent)
{
    std::string actionId;
    if (!cxx::json_get_attribute(actionNode, "action", actionId))
    {
        debug_assert(false);
        return nullptr;
    }

    GuiWidgetAction* action = nullptr;
    if (actionId == "show")
    {
        action = new GuiWidgetActionShow(actionsParent);
    }
    else if (actionId == "size")
    {
        action = new GuiWidgetActionSize(actionsParent);
    }
    else if (actionId == "texture")
    {
        action = new GuiWidgetActionSetTexture(actionsParent);
    }
    else if (actionId == "emit_event")
    {
        action = new GuiWidgetActionEmitEvent(actionsParent);
    }

    debug_assert(action);
    if (action)
    {
        if (!action->Deserialize(actionNode))
        {
            debug_assert(false);

            action->ReleaseAction();
            action = nullptr;
        }
    }
    return action;
}
