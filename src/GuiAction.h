#pragma once

#include "GuiDefs.h"

// forwards
class GuiAction;
class GuiActionsHolder;

//////////////////////////////////////////////////////////////////////////

class GuiActionContext
{
public:
    virtual ~GuiActionContext()
    {
    }
    // try to resolve single condition by name
    // @param name: Identifier name
    // @param isTrue: Output result    
    virtual bool ResolveCondition(const GuiWidget* source, const cxx::unique_string& name, bool& isTrue) = 0;
};

//////////////////////////////////////////////////////////////////////////

// base widget action class
class GuiAction: public cxx::noncopyable
{
public:
    void PerformAction(GuiWidget* parentWidget, const GuiEvent& eventData);
    void ReleaseAction();
    bool Deserialize(cxx::json_node_object actionNode);
    GuiAction* CloneAction();

protected:
    GuiAction() = default;
    GuiAction(const GuiAction* copyAction);
    virtual ~GuiAction()
    {
    }

    bool EvaluateConditions(GuiWidget* parentWidget, const GuiEvent& eventData) const;

protected:
    // overridables
    virtual void HandlePerformAction(GuiWidget* targetWidget) = 0;
    virtual void HandlePerformActionOnConditionsFail(GuiWidget* targetWidget) {}
    virtual bool HandleDeserialize(cxx::json_node_object actionNode)
    {
        return true;
    }
    virtual GuiAction* HandleCloneAction() = 0;

protected:
    // common properties
    cxx::logical_expression mConditions;
    std::string mTargetPath;
};

//////////////////////////////////////////////////////////////////////////

// widget actions holder
class GuiActionsHolder: public cxx::noncopyable
{
public:
    // readonly
    GuiWidget* mParentWidget;

public:
    GuiActionsHolder(GuiWidget* actionsParentWidget);
    ~GuiActionsHolder();
    void CopyActions(const GuiActionsHolder& source);

    // add action to controller
    void AddAction(cxx::unique_string eventId, GuiAction* action);
    void ClearActions();

    // invoke actions associated with event id
    void EmitEvent(const GuiEvent& eventData);

private:
    struct EventActionStruct
    {
    public:
        cxx::unique_string mEventId;
        GuiAction* mAction; // action associated with event
    };
    std::vector<EventActionStruct> mActionsList;
};

//////////////////////////////////////////////////////////////////////////

// actions manager
class GuiActionsFactory: public cxx::noncopyable
{
public:
    // try load single widget action from json document node
    // @returns null on error
    GuiAction* DeserializeAction(cxx::json_node_object actionNode);
};

extern GuiActionsFactory gGuiActionsFactory;