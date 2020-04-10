#pragma once

#include "GuiWidget.h"
#include "GuiEvent.h"

class GuiManager: public cxx::noncopyable
{
public:
    // readonly
    GuiDragDropHandler* mCurrentDragHandler = nullptr;
    GuiWidget* mHoveredWidget = nullptr;

public:
    GuiManager();

    // setup gui manager internal resources
    bool Initialize();
    void Deinit();

    // render
    void RenderFrame(GuiRenderer& renderContext);

    // process single frame logic
    void UpdateFrame();

    // process input event
    // @param inputEvent: Event data
    void HandleInputEvent(MouseButtonInputEvent& inputEvent);
    void HandleInputEvent(MouseMovedInputEvent& inputEvent);
    void HandleInputEvent(MouseScrollInputEvent& inputEvent);
    void HandleInputEvent(KeyInputEvent& inputEvent);
    void HandleInputEvent(KeyCharEvent& inputEvent);

    // register specific widget class in gui system
    // @returns false if class name is not unique or factory does not specified for this class
    bool RegisterWidgetClass(GuiWidgetClass* widgetsClass);

    // find widget class by its name
    // @param className: Widget class name
    GuiWidgetClass* GetWidgetClass(const std::string& className) const;

    // construct new widget of specified class, if it registered
    // @param className: Widget class name
    GuiWidget* ConstructWidget(const std::string& className) const;

    // set active drag handler
    // @param dragHandler: New handler or null to cancel current drag
    // @param screenPoint: Start drag position in screen coordinates
    void SetDragHandler(GuiDragDropHandler* dragHandler, const Point& screenPoint);
    
    // process screen resolution changed event
    void HandleScreenResolutionChanged();

    // process widget being destroyed event
    // @param widget: Target widget
    void HandleWidgetDestroy(GuiWidget* widget);

    // push event to events queue and notify registered handlers at beginning of next frame
    // @param eventData: source event data
    void PostGuiEvent(GuiEvent* eventData);

    // add or remove gui events handler, usually these methods are not explicitly called
    // @param eventsHandler: Handler
    void RegisterEventsHandler(GuiEventsHandler* eventsHandler);
    void UnregisterEventsHandler(GuiEventsHandler* eventsHandler);

    // temporary
    // todo: remove
    void AttachWidget(GuiWidget* widget);
    void DetachWidget(GuiWidget* widget);

private:
    void RegisterWidgetsClasses();

    void HandleMouseLButtonPressed(MouseButtonInputEvent& inputEvent);
    void HandleMouseLButtonReleased(MouseButtonInputEvent& inputEvent);

    void UpdateCurrentHoveredWidget();
    void ProcessEventsQueue();
    void ClearEventsQueue();

private:
    using GuiWidgetClassesMap = std::unordered_map<std::string, GuiWidgetClass*>;
    GuiWidgetClassesMap mWidgetsClasses;

    std::vector<GuiEventsHandler*> mEventHandlers;
    std::vector<GuiEvent> mEventsQueue;
    std::vector<GuiEvent> mProcessingEventsQueue;

    std::vector<GuiWidget*> mWidgets; // temporary: todo: remove
};

extern GuiManager gGuiManager;