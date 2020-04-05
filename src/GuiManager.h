#pragma once

#include "GuiWidget.h"

class GuiManager: public cxx::noncopyable
{
public:
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

    void HandleScreenResolutionChanged();

    // temporary
    // todo: remove
    void AttachWidget(GuiWidget* widget);
    void DetachWidget(GuiWidget* widget);

private:
    void RegisterWidgetsClasses();

private:
    using GuiWidgetClassesMap = std::unordered_map<std::string, GuiWidgetClass*>;
    GuiWidgetClassesMap mWidgetsClasses;

    std::vector<GuiWidget*> mWidgets; // temporary: todo: remove
};

extern GuiManager gGuiManager;