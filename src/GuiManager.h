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
    bool RegisterWidgetClass(GuiWidgetClass* widgetsClass);

    // find widget class by its name
    // @param className: Widget class name
    GuiWidgetClass* GetWidgetClass(const std::string& className) const;

    // construct new widget of specified class, if it registered
    // @param className: Widget class name
    GuiWidget* ConstructWidget(const std::string& className) const;

    // construct new template widget of specified class, if it registered
    // @param className: Template widget class name
    GuiWidget* ConstructTemplateWidget(const std::string& templateClassName) const;

    // start listen mouse events
    void CaptureMouseInputs(GuiWidget* mouseListener);
    void ClearMouseCapture();
    
    // process screen resolution changed event
    void HandleScreenResolutionChanged();

    // push event to events queue and notify registered handlers at beginning of next frame
    // @param eventData: source event data
    void PostGuiEvent(const GuiEvent& eventData);

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

    bool LoadTemplateWidgets();
    void FreeTemplateWidgets();

    void ScanHoveredWidget();
    void ProcessEventsQueue();
    void ClearEventsQueue();

private:
    using GuiWidgetClassesMap = std::unordered_map<std::string, GuiWidgetClass*>;
    GuiWidgetClassesMap mWidgetsClasses;

    using GuiTemplateWidgetsMap = std::unordered_map<std::string, GuiWidget*>;
    GuiTemplateWidgetsMap mTemplateWidgetsClasses;

    std::vector<GuiEventsHandler*> mEventHandlers;
    std::vector<GuiEvent> mEventsQueue;
    std::vector<GuiEvent> mProcessingEventsQueue;
    std::vector<GuiHierarchy*> mHiersList;
};

extern GuiManager gGuiManager;