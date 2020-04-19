#pragma once

#include "GuiWidget.h"
#include "GuiEvent.h"

class GuiManager: public cxx::noncopyable
{
public:
    // readonly
    cxx::handle<GuiWidget> mMouseCaptureWidget;
    cxx::handle<GuiWidget> mHoveredWidget;

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
    bool RegisterWidgetClass(GuiWidgetMetaClass* widgetsClass);

    // find widget class by its name
    // @param className: Widget class name
    GuiWidgetMetaClass* GetWidgetClass(cxx::unique_string className) const;

    // construct new widget of specified class, if it registered
    // @param className: Widget class name
    GuiWidget* ConstructWidget(cxx::unique_string className) const;

    // start listen mouse events
    void CaptureMouseInputs(GuiWidget* mouseListener);
    void ClearMouseCapture();
    
    // process screen resolution changed event
    void HandleScreenResolutionChanged();

    // push event to events queue and notify registered handlers at beginning of next frame
    // @param eventData: source event data
    void BroadcastEvent(const GuiEvent& eventData);

    // add or remove gui events handler, usually these methods are not explicitly called
    // @param eventsHandler: Handler
    void RegisterEventsHandler(GuiEventsHandler* eventsHandler);
    void UnregisterEventsHandler(GuiEventsHandler* eventsHandler);
    void UnregisterAllEventsHandlers();

    // attach/detach widgets
    // @param hierarchy: Hierarchy
    void AttachWidgets(GuiHierarchy* hierarchy);
    void DetachWidgets(GuiHierarchy* hierarchy);
    void DetachAllWidgets();

private:
    void RegisterWidgetsClasses();
    void UnregisterWidgetsClasses();

    void ScanHoveredWidget();
    void ProcessEventsQueue();
    void ClearEventsQueue();

    bool IsProcessingEvents() const;

private:
    using GuiWidgetClassesMap = std::map<cxx::unique_string, GuiWidgetMetaClass*>;
    GuiWidgetClassesMap mWidgetsClasses;

    std::vector<GuiEventsHandler*> mEventHandlers;
    std::vector<GuiEvent> mEventsQueue;
    std::vector<GuiEvent> mProcessingEventsQueue;
    std::vector<GuiHierarchy*> mHiersList;
};

extern GuiManager gGuiManager;