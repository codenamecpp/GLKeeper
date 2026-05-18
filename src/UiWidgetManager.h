#pragma once

//////////////////////////////////////////////////////////////////////////

#include "UiDefs.h"
#include "UiWidget.h"

//////////////////////////////////////////////////////////////////////////

class UiWidgetManager
{
    friend class UiWidget;

public:
    bool Initialize();
    void Shutdown();

    void RenderFrame(UiRenderContext& renderContext);
    void UpdateFrame(float deltaTime);

    // Process input events
    // @param inputEvent: Input event data
    void InputEvent(MouseMovedInputEvent& inputEvent);
    void InputEvent(MouseScrollInputEvent& inputEvent);
    void InputEvent(MouseButtonInputEvent& inputEvent);
    void InputEvent(KeyInputEvent& inputEvent);
    void InputEvent(KeyCharEvent& inputEvent);

    // Register / Unregister specific widget class
    // @returns false if class name is not unique or factory does not specified for this class
    bool RegisterWidgetClass(const std::string& widgetClassName, UiWidget* prototype);

    // Find widget class by its name
    // @param className: Widget class name
    UiWidget* GetWidgetClassPrototype(const std::string& className) const;

    // Construct widget of specified class, if it class registered
    UiWidget* ConstructWidget(const std::string& className) const;

    template<typename TWidget>
    inline TWidget* ConstructWidget() const
    {
        const std::string typeName = typeid(TWidget).name();
        auto map_it = mWidgetTypeToClassName.find(typeName);
        if (map_it != mWidgetTypeToClassName.end())
        {
            return static_cast<TWidget*>(ConstructWidget(map_it->second));
        }
        cxx_assert(false);
        return nullptr;
    }

    // Set currently focused widget - it will receive input events even it not hovered
    // @param focusWidget: Target widget
    void CaptureFocus(UiWidget* focusWidget);

    // Cancel focus for widget
    // @param focusWidget: Target widget
    void ReleaseFocus(UiWidget* focusWidget);

    // Get currently hovered and focused widgets
    inline UiWidget* GetHoveredWidget() const { return mHoveredWidget; }
    inline UiWidget* GetFocusedWidget() const { return mFocusedWidget; }

    // views management
    void AttachView(UiView* view);
    void DetachView(UiView* view);
    void DetachAllViews();
    bool ViewAttached(const UiView* view) const;

    // Test whether cursor is over ui element
    bool IsCursorOverUI() const;

public:
    // notifications
    void ScreenSizeChanged();

private:
    template<typename TWidget>
    bool RegisterWidgetClass();

    void MarkWidgetForDelete(UiWidget* widget);
    void ProcessWidgetToDelete();

    void RegisterWidgetClasses();
    void UpdateCurrentHovered();
    void UpdateCurrentFocused();

    // Change current hovered widget
    void SetHoverWidget(UiWidget* hoverWidget);

private:
    std::unordered_map<std::string, UiWidget*> mWidgetClassPrototypes;
    std::unordered_map<std::string, std::string> mWidgetTypeToClassName;

    UiWidget* mHoveredWidget = nullptr;
    UiWidget* mFocusedWidget = nullptr;

    std::vector<UiView*> mViews;

    std::vector<UiWidget*> mWidgetsToDelete;
};

extern UiWidgetManager gWidgetManager;
