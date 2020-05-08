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
    void ProcessInputEvent(MouseButtonInputEvent& inputEvent);
    void ProcessInputEvent(MouseMovedInputEvent& inputEvent);
    void ProcessInputEvent(MouseScrollInputEvent& inputEvent);
    void ProcessInputEvent(KeyInputEvent& inputEvent);
    void ProcessInputEvent(KeyCharEvent& inputEvent);

    // register specific widget class in gui system
    // @returns false if class name is not unique or factory does not specified for this class
    bool RegisterWidgetClass(GuiWidgetMetaClass* widgetsClass);

    // find widget class by its name
    // @param className: Widget class name
    GuiWidgetMetaClass* GetWidgetClass(cxx::unique_string className) const;

    // construct new widget of specified class, if it registered
    // @param className: Widget class name
    GuiWidget* CreateWidget(cxx::unique_string className) const;

    // start listen mouse events
    void SetMouseCapture(GuiWidget* widget);
    void ReleaseMouseCapture(GuiWidget* widget);
    
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

    // attach/detach screen layers
    void AttachScreen(GuiScreen* screen);
    void DetachScreen(GuiScreen* screen);
    void DetachAllScreens();

    // get content location for specific gui screen
    bool GetScreenContentPath(cxx::unique_string screenId, std::string& contentPath);

private:
    void RegisterWidgetsClasses();
    void UnregisterWidgetsClasses();

    void UpdateHoveredWidget();
    void UpdateMouseCaptureWidget();
    void ProcessEventsQueue();
    void ClearEventsQueue();

    bool IsProcessingEvents() const;
    
    void LoadScreenRecords();
    void FreeScreenRecords();

private:
    using GuiWidgetClassesMap = std::map<cxx::unique_string, GuiWidgetMetaClass*>;
    GuiWidgetClassesMap mWidgetsClasses;

    std::vector<GuiEventsHandler*> mEventHandlers;
    std::vector<GuiEvent> mEventsQueue;
    std::vector<GuiEvent> mProcessingEventsQueue;

    struct ScreenElement
    {
    public:
        cxx::unique_string mScreenId;
        std::string mContentPath;
        GuiScreen* mInstance = nullptr;
    };
    std::vector<ScreenElement> mScreensList;
};

extern GuiManager gGuiManager;