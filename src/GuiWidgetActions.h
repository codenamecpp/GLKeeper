#pragma once

#include "GuiDefs.h"

// forwards
class GuiWidgetAction;
class GuiWidgetActionsList;

//////////////////////////////////////////////////////////////////////////

// base widget action class
class GuiWidgetAction: public cxx::noncopyable
{
public:
    void PerformAction();
    void ReleaseAction();
    bool Deserialize(cxx::json_node_object actionNode);
    GuiWidgetAction* CloneAction(GuiWidget* parentWidget);

protected:
    GuiWidgetAction(GuiWidget* parentWidget);
    GuiWidgetAction(GuiWidget* parentWidget, const GuiWidgetAction* copyAction);
    virtual ~GuiWidgetAction()
    {
    }

    bool EvaluateConditions() const;

protected:
    // overridables
    virtual void HandlePerformAction(GuiWidget* targetWidget) = 0;
    virtual bool HandleDeserialize(cxx::json_node_object actionNode)
    {
        return true;
    }
    virtual GuiWidgetAction* HandleCloneAction(GuiWidget* parentWidget) = 0;

protected:
    GuiWidget* mParentWidget; // mandatory

    cxx::logical_expression mConditions; // optional
    std::string mTargetPath; // optional
};

//////////////////////////////////////////////////////////////////////////

// widget actions list
class GuiWidgetActionsList: public cxx::noncopyable
{
public:
    // readonly
    GuiWidget* mParentWidget;

public:
    GuiWidgetActionsList(GuiWidget* actionsParentWidget);
    ~GuiWidgetActionsList();

    // add action to controller
    void AddAction(cxx::unique_string eventId, GuiWidgetAction* action);
    void ClearActions();
    void CopyActions(const GuiWidgetActionsList& source);

    // invoke actions associated with event id
    void InvokeEventActions(const GuiEvent& eventData);

private:
    struct EventActionStruct
    {
    public:
        cxx::unique_string mEventId;
        GuiWidgetAction* mAction; // action associated with event
    };

    std::vector<EventActionStruct> mActionsList;
};

//////////////////////////////////////////////////////////////////////////

// actions manager
class GuiWidgetActionsManager: public cxx::noncopyable
{
public:
    // try load widget actions from json document node
    void DeserializeActions(cxx::json_node_object actionsNode, GuiWidgetActionsList& actionsList);

    // try load single widget action from json document node
    GuiWidgetAction* DeserializeAction(cxx::json_node_object actionNode, GuiWidget* actionsParent);
};

extern GuiWidgetActionsManager gGuiWidgetActionsManager;