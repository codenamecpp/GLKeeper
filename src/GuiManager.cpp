#include "pch.h"
#include "GuiManager.h"
#include "GuiRenderer.h"
#include "Console.h"
#include "TimeManager.h"
#include "GuiPictureBox.h"
#include "InputsManager.h"

GuiManager gGuiManager;

bool GuiManager::Initialize()
{
    RegisterWidgetsClasses();
    return true;
}

void GuiManager::Deinit()
{
    mWidgetsClasses.clear();
    mWidgets.clear();
}

void GuiManager::AttachWidget(GuiWidget* widget)
{
    cxx::push_back_if_unique(mWidgets, widget);
}

void GuiManager::DetachWidget(GuiWidget* widget)
{
    cxx::erase_elements(mWidgets, widget);
}

void GuiManager::RenderFrame(GuiRenderer& renderContext)
{
    for (GuiWidget* currWidget: mWidgets)
    {
        currWidget->RenderFrame(renderContext);
    }
}

void GuiManager::UpdateFrame()
{
    float deltaTime = (float) gTimeManager.GetRealtimeFrameDelta();

    for (GuiWidget* currWidget: mWidgets)
    {
        currWidget->UpdateFrame(deltaTime);
    }
}

void GuiManager::HandleInputEvent(MouseButtonInputEvent& inputEvent)
{
    if (inputEvent.mButton == eMouseButton_Left)
    {
        if (inputEvent.mPressed)
        {
            HandleMouseLButtonPressed(inputEvent);
        }
        else
        {
            HandleMouseLButtonReleased(inputEvent);
        }
    }
}

void GuiManager::HandleInputEvent(MouseMovedInputEvent& inputEvent)
{
    if (mDraggedWidget) // continue drag
    {
        mDraggedWidget->HandleDrag(gInputsManager.mCursorPosition);

        inputEvent.SetConsumed();
    }
}

void GuiManager::HandleInputEvent(MouseScrollInputEvent& inputEvent)
{
}

void GuiManager::HandleInputEvent(KeyInputEvent& inputEvent)
{
}

void GuiManager::HandleInputEvent(KeyCharEvent& inputEvent)
{
}

void GuiManager::HandleMouseLButtonPressed(MouseButtonInputEvent& inputEvent)
{
}

void GuiManager::HandleMouseLButtonReleased(MouseButtonInputEvent& inputEvent)
{
    if (mDraggedWidget) // finish drag
    {
        mDraggedWidget->HandleDragDrop(gInputsManager.mCursorPosition);

        inputEvent.SetConsumed();
    }
}

bool GuiManager::RegisterWidgetClass(GuiWidgetClass* widgetsClass)
{
    debug_assert(widgetsClass);
    if (widgetsClass == nullptr || widgetsClass->mFactory == nullptr)
        return false;

    GuiWidgetClass*& classRegistered = mWidgetsClasses[widgetsClass->mClassName];
    if (classRegistered)
    {
        gConsole.LogMessage(eLogMessage_Warning, "Widget class '%s' already registered in gui manager", widgetsClass->mClassName.c_str());
        return false;
    }
    classRegistered = widgetsClass;
    return true;
}

void GuiManager::RegisterWidgetsClasses()
{
    RegisterWidgetClass(&gBaseWidgetClass);
    RegisterWidgetClass(&gPictureBoxWidgetClass);
}

GuiWidgetClass* GuiManager::GetWidgetClass(const std::string& className) const
{
    auto iterator_found = mWidgetsClasses.find(className);
    if (iterator_found == mWidgetsClasses.end())
        return nullptr;

    return iterator_found->second;
}

GuiWidget* GuiManager::ConstructWidget(const std::string& className) const
{
    if (GuiWidgetClass* widgetClass = GetWidgetClass(className))
    {
        debug_assert(widgetClass->mFactory);
        return widgetClass->mFactory->ConstructWidget();
    }

    debug_assert(false);
    return nullptr;
}

void GuiManager::StartDrag(GuiWidget* widget)
{
    debug_assert(widget);
    if (widget == nullptr || mDraggedWidget == widget)
        return;

    CancelDrag();

    mDraggedWidget = nullptr;
    if (widget->HandleDragStart(gInputsManager.mCursorPosition))
    {
        mDraggedWidget = widget;
    }
}

void GuiManager::CancelDrag()
{
    if (mDraggedWidget)
    {
        mDraggedWidget->HandleDragCancel();
        mDraggedWidget = nullptr;
    }
}

void GuiManager::HandleScreenResolutionChanged()
{
}