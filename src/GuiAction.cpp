#include "pch.h"
#include "GuiAction.h"
#include "GuiWidget.h"
#include "GuiHelpers.h"
#include "GuiEvent.h"
#include "GuiPictureBox.h"

GuiActionsFactory gGuiActionsFactory;

//////////////////////////////////////////////////////////////////////////

GuiAction::GuiAction(const GuiAction* copyAction)
    : mConditions(copyAction->mConditions)
    , mTargetPath(copyAction->mTargetPath)
{
}

void GuiAction::PerformAction(GuiWidget* parentWidget)
{
    if (!EvaluateConditions(parentWidget))
        return;

    GuiWidget* target = parentWidget;
    if (!mTargetPath.empty())
    {
        target = GuiGetChildWidgetByPath(parentWidget, mTargetPath);
    }
    debug_assert(target);
    if (target)
    {
        HandlePerformAction(target);
    }
}

void GuiAction::ReleaseAction()
{
    delete this;
}

bool GuiAction::Deserialize(cxx::json_node_object actionNode)
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

GuiAction* GuiAction::CloneAction()
{
    return HandleCloneAction();
}

bool GuiAction::EvaluateConditions(GuiWidget* parentWidget) const
{
    debug_assert(parentWidget);

    bool isTrue = true;
    if (mConditions.non_null())
    {
        isTrue = mConditions.evaluate_expression([parentWidget](const cxx::unique_string& name)
        {
            bool condition;
            return parentWidget->ResolveCondition(name, condition) && condition;
        });
    }
    return isTrue;
}

//////////////////////////////////////////////////////////////////////////

class GuiWidgetActionShow: public GuiAction
{
public:
    GuiWidgetActionShow() = default;
    GuiWidgetActionShow(const GuiWidgetActionShow* copyAction)
        : GuiAction(copyAction)
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
    GuiWidgetActionShow* HandleCloneAction() override
    {
        return new GuiWidgetActionShow(this);
    }
public:
    bool mIsShow = true;
};

//////////////////////////////////////////////////////////////////////////

class GuiWidgetActionSize: public GuiAction
{
public:
    GuiWidgetActionSize() = default;
    GuiWidgetActionSize(const GuiWidgetActionSize* copyAction)
        : GuiAction(copyAction)
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
    GuiWidgetActionSize* HandleCloneAction() override
    {
        return new GuiWidgetActionSize(this);
    }
private:
    eGuiUnits mUnitsW;
    eGuiUnits mUnitsH;
    Point mSize;
};

//////////////////////////////////////////////////////////////////////////

class GuiWidgetActionSetTexture: public GuiAction
{
public:
    GuiWidgetActionSetTexture() = default;
    GuiWidgetActionSetTexture(const GuiWidgetActionSetTexture* copyAction)
        : GuiAction(copyAction)
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
    GuiWidgetActionSetTexture* HandleCloneAction() override
    {
        return new GuiWidgetActionSetTexture(this);
    }
private:
    std::string mTextureName;
};

//////////////////////////////////////////////////////////////////////////

class GuiWidgetActionEmitEvent: public GuiAction
{
public:
    GuiWidgetActionEmitEvent() = default;
    GuiWidgetActionEmitEvent(const GuiWidgetActionEmitEvent* copyAction)
        : GuiAction(copyAction)
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
        targetWidget->mActions.EmitEvent(mEventId);
    }
    bool HandleDeserialize(cxx::json_node_object actionNode) override
    {
        cxx::json_get_attribute(actionNode, "value", mEventId);
        debug_assert(!mEventId.empty());
        return true;
    }
    GuiWidgetActionEmitEvent* HandleCloneAction() override
    {
        return new GuiWidgetActionEmitEvent(this);
    }
private:
    cxx::unique_string mEventId;
};

//////////////////////////////////////////////////////////////////////////

class GuiWidgetActionBackgroundColor: public GuiAction
{
public:
    GuiWidgetActionBackgroundColor() = default;
    GuiWidgetActionBackgroundColor(const GuiWidgetActionBackgroundColor* copyAction)
        : GuiAction(copyAction)
        , mBackgroundColor(copyAction->mBackgroundColor)
    {
    }
    void HandlePerformAction(GuiWidget* targetWidget) override
    {
        targetWidget->mBackgroundColor = mBackgroundColor;
    }
    bool HandleDeserialize(cxx::json_node_object actionNode) override
    {
        if (!GuiParseColor(actionNode["value"], mBackgroundColor))
            return false;

        return true;
    }
    GuiWidgetActionBackgroundColor* HandleCloneAction() override
    {
        return new GuiWidgetActionBackgroundColor(this);
    }
private:
    Color32 mBackgroundColor = Color32_Gray;
};

//////////////////////////////////////////////////////////////////////////

GuiActionsHolder::GuiActionsHolder(GuiWidget* actionsParentWidget)
    : mParentWidget(actionsParentWidget)
{
    debug_assert(mParentWidget);
}

GuiActionsHolder::~GuiActionsHolder()
{
    ClearActions();
}

void GuiActionsHolder::AddAction(cxx::unique_string eventId, GuiAction* action)
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

void GuiActionsHolder::ClearActions()
{
    for (const EventActionStruct& curr: mActionsList)
    {
        curr.mAction->ReleaseAction();
    }
    mActionsList.clear();
}

void GuiActionsHolder::EmitEvent(cxx::unique_string eventId)
{
    for (const EventActionStruct& curr: mActionsList)
    {
        if (curr.mEventId == eventId)
        {
            curr.mAction->PerformAction(mParentWidget);
        }
    }
}

void GuiActionsHolder::CopyActions(const GuiActionsHolder& source)
{
    for (const EventActionStruct& curr_element: source.mActionsList)
    {
        GuiAction* action = curr_element.mAction->CloneAction();
        debug_assert(action);

        if (action)
        {
            AddAction(curr_element.mEventId, action);
        }
    }
}

//////////////////////////////////////////////////////////////////////////

GuiAction* GuiActionsFactory::DeserializeAction(cxx::json_node_object actionNode)
{
    std::string actionId;
    if (!cxx::json_get_attribute(actionNode, "action", actionId))
    {
        debug_assert(false);
        return nullptr;
    }
    // todo: refactore this if/else mess
    GuiAction* action = nullptr;
    if (actionId == "show")
    {
        action = new GuiWidgetActionShow;
    }
    else if (actionId == "size")
    {
        action = new GuiWidgetActionSize;
    }
    else if (actionId == "texture")
    {
        action = new GuiWidgetActionSetTexture;
    }
    else if (actionId == "emit_event")
    {
        action = new GuiWidgetActionEmitEvent;
    }
    else if (actionId == "background_color")
    {
        action = new GuiWidgetActionBackgroundColor;
    }

    debug_assert(action);
    if (action)
    {
        if (!action->Deserialize(actionNode))
        {
            action->ReleaseAction();
            action = nullptr;
        }
        debug_assert(action);
    }
    return action;
}
