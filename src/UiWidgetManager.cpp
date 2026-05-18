#include "stdafx.h"
#include "UiWidgetManager.h"
#include "UiRenderContext.h"
#include "UiWidget.h"
#include "UiPicture.h"
#include "UiButton.h"
#include "UiProductionButton.h"
#include "UiView.h"
#include "UiPanel.h"
#include "UiTextBox.h"
#include "UiGridLayout.h"

UiWidgetManager gWidgetManager;

bool UiWidgetManager::Initialize()
{
    RegisterWidgetClasses();

    return true;
}

void UiWidgetManager::Shutdown()
{
    mHoveredWidget = nullptr;
    mFocusedWidget = nullptr;

    DetachAllViews();

    // cleanup prototypes
    for (const auto& roller: mWidgetClassPrototypes)
    {
        UiWidget* prototype = roller.second;
        MarkWidgetForDelete(prototype);
    }
    mWidgetClassPrototypes.clear();
    mWidgetTypeToClassName.clear();

    // free memory
    ProcessWidgetToDelete();
}

void UiWidgetManager::MarkWidgetForDelete(UiWidget* widget)
{
    if ((widget == nullptr) || cxx::contains(mWidgetsToDelete, widget))
    {
        cxx_assert(false);
        return;
    }
    mWidgetsToDelete.push_back(widget);
    
    if (widget == mHoveredWidget)
    {
        mHoveredWidget = nullptr;
    }

    if (widget == mFocusedWidget)
    {
        mFocusedWidget = nullptr;
    }
}

void UiWidgetManager::ProcessWidgetToDelete()
{
    if (mWidgetsToDelete.empty()) 
        return;

    for (UiWidget* roller: mWidgetsToDelete)
    {
        delete roller;
    }
    mWidgetsToDelete.clear();
}

void UiWidgetManager::RegisterWidgetClasses()
{
    RegisterWidgetClass<UiWidget>();
    RegisterWidgetClass<UiPicture>();
    RegisterWidgetClass<UiButton>();
    RegisterWidgetClass<UiPanel>();
    RegisterWidgetClass<UiProductionButton>();
    RegisterWidgetClass<UiTextBox>();
    RegisterWidgetClass<UiGridLayout>();
}

template<typename TWidget>
bool UiWidgetManager::RegisterWidgetClass()
{
    TWidget* widgetInstance = new TWidget;
    bool isSuccess = RegisterWidgetClass(widgetInstance->mClassName, widgetInstance);
    if (!isSuccess)
    {
        widgetInstance->DeleteWidget();
    }
    return isSuccess;
}

void UiWidgetManager::InputEvent(MouseMovedInputEvent& inputEvent)
{
    if (mFocusedWidget)
    {
        mFocusedWidget->InputEvent(inputEvent);
        inputEvent.SetConsumed(true); // force
        return;
    }

    if (mHoveredWidget)
    {
        mHoveredWidget->InputEvent(inputEvent);
        return;
    }

    // process interactive layers in reverse order
    for (auto it = mViews.rbegin(); it != mViews.rend(); ++it)
    {
        UiView* currentView = *it;
        currentView->InputEvent(inputEvent);
        if (inputEvent.mConsumed)
            return;
    }

    if (IsCursorOverUI())
    {
        inputEvent.SetConsumed(true);
    }
}

void UiWidgetManager::InputEvent(MouseScrollInputEvent& inputEvent)
{
    if (mFocusedWidget)
    {
        mFocusedWidget->InputEvent(inputEvent);
        return;
    }

    if (mHoveredWidget)
    {
        mHoveredWidget->InputEvent(inputEvent);
        return;
    }

    // process interactive layers in reverse order
    for (auto it = mViews.rbegin(); it != mViews.rend(); ++it)
    {
        UiView* currentView = *it;
        currentView->InputEvent(inputEvent);
        if (inputEvent.mConsumed)
            return;
    }

    if (IsCursorOverUI())
    {
        inputEvent.SetConsumed(true);
    }
}

void UiWidgetManager::InputEvent(MouseButtonInputEvent& inputEvent)
{
    if (mFocusedWidget)
    {
        mFocusedWidget->InputEvent(inputEvent);
        return;
    }

    if (mHoveredWidget)
    {
        mHoveredWidget->InputEvent(inputEvent);
        return;
    }

    // process interactive layers in reverse order
    for (auto it = mViews.rbegin(); it != mViews.rend(); ++it)
    {
        UiView* currentView = *it;
        currentView->InputEvent(inputEvent);
        if (inputEvent.mConsumed)
            return;
    }

    if (IsCursorOverUI())
    {
        inputEvent.SetConsumed(true);
    }
}

void UiWidgetManager::InputEvent(KeyCharEvent& inputEvent)
{
    if (mFocusedWidget)
    {
        mFocusedWidget->InputEvent(inputEvent);
        if (inputEvent.mConsumed)
            return;
    }

    if (mHoveredWidget)
    {
        mHoveredWidget->InputEvent(inputEvent);
        if (inputEvent.mConsumed)
            return;
    }

    // process interactive layers in reverse order
    for (auto it = mViews.rbegin(); it != mViews.rend(); ++it)
    {
        UiView* currentView = *it;
        currentView->InputEvent(inputEvent);
        if (inputEvent.mConsumed)
            return;
    }
}

void UiWidgetManager::InputEvent(KeyInputEvent& inputEvent)
{
    if (mFocusedWidget)
    {
        mFocusedWidget->InputEvent(inputEvent);
        if (inputEvent.mConsumed)
            return;
    }

    if (mHoveredWidget)
    {
        mHoveredWidget->InputEvent(inputEvent);
        if (inputEvent.mConsumed)
            return;
    }

    // process interactive layers in reverse order
    for (auto it = mViews.rbegin(); it != mViews.rend(); ++it)
    {
        UiView* currentView = *it;
        currentView->InputEvent(inputEvent);
        if (inputEvent.mConsumed)
            return;
    }
}

void UiWidgetManager::RenderFrame(UiRenderContext& renderContext)
{
    // render interactive ui layers
    for (UiView* roller: mViews)
    {
        renderContext.SetTransform(nullptr);
        roller->RenderFrame(renderContext);
    }
}

void UiWidgetManager::UpdateFrame(float deltaTime)
{
    ProcessWidgetToDelete();

    // update interactive ui layers
    for (UiView* roller: mViews)
    {
        roller->UpdateFrame(deltaTime);
    }

    UpdateCurrentHovered();
    UpdateCurrentFocused();
}

void UiWidgetManager::UpdateCurrentHovered()
{
    UiWidget* currentHovered = nullptr;

    if (!mViews.empty())
    {
        // find current hovered widget at top-most layer only
        UiView* currentView = mViews.back();
        UiWidget* hierarchyRoot = currentView->GetHierarchy().GetRootWidget();
        // cannot pick widget of invisible uistate
        if (hierarchyRoot)
        {
            currentHovered = hierarchyRoot->PickWidget({gInputs.mCursorPositionX, gInputs.mCursorPositionY});
        }
    }
    SetHoverWidget(currentHovered);
}

void UiWidgetManager::UpdateCurrentFocused()
{
    // release focus for hidden or non-interactive widget
    if (mFocusedWidget)
    {
        if (!mFocusedWidget->IsVisibleInHierarchy() ||
            !mFocusedWidget->IsEnabledInHierarchy() || 
            !mFocusedWidget->IsInteractive())
        {
            ReleaseFocus(mFocusedWidget);
        }
    }
}

void UiWidgetManager::SetHoverWidget(UiWidget* hoverWidget)
{
    if (mHoveredWidget == hoverWidget)
        return;

    UiWidget* prevHovered = mHoveredWidget;
    mHoveredWidget = hoverWidget;

    if (prevHovered)
    {
        prevHovered->HandleMouseLeave();
    }

    if (mHoveredWidget)
    {
        mHoveredWidget->HandleMouseEnter();
    }
}

UiWidget* UiWidgetManager::ConstructWidget(const std::string& className) const
{
    UiWidget* instance = nullptr;
    if (UiWidget* prototype = GetWidgetClassPrototype(className))
    {
        instance = prototype->CloneWidget();
    }
    cxx_assert(instance);
    return instance;
}

void UiWidgetManager::CaptureFocus(UiWidget* focusWidget)
{
    cxx_assert(focusWidget);
    if (mFocusedWidget)
    {
        ReleaseFocus(mFocusedWidget);
    }

    mFocusedWidget = focusWidget;
    mFocusedWidget->HandleFocusGained();
}

void UiWidgetManager::ReleaseFocus(UiWidget* focusWidget)
{
    cxx_assert(focusWidget);
    if (focusWidget && focusWidget == mFocusedWidget)
    {
        mFocusedWidget = nullptr;
        focusWidget->HandleFocusLost();
    }
}

void UiWidgetManager::AttachView(UiView* view)
{
    cxx_assert(view);
    bool canAppend = std::find(mViews.begin(), mViews.end(), view) == mViews.end();
    if (canAppend && view)
    {
        // find right place to insert
        auto insertPosition = std::find_if(mViews.begin(), mViews.end(), 
            [view](UiView* currentLayer)
            {
                return currentLayer->mViewLayer > view->mViewLayer;
            });

        mViews.insert(insertPosition, view);
    }
    cxx_assert(canAppend);
}

void UiWidgetManager::DetachView(UiView* view)
{
    auto views_it = std::find(mViews.begin(), mViews.end(), view);
    cxx_assert(views_it != mViews.end());
    if (views_it != mViews.end())
    {
        mViews.erase(views_it);
    }
}

void UiWidgetManager::DetachAllViews()
{
    mViews.clear();
}

bool UiWidgetManager::ViewAttached(const UiView* view) const
{
    auto views_it = std::find(mViews.begin(), mViews.end(), view);
    return views_it != mViews.end();
}

bool UiWidgetManager::IsCursorOverUI() const
{
    return mHoveredWidget != nullptr;
}

void UiWidgetManager::ScreenSizeChanged()
{
    for (UiView* roller: mViews)
    {
        roller->ResolutionChanged();
    }
}

bool UiWidgetManager::RegisterWidgetClass(const std::string& widgetClassName, UiWidget* prototype)
{
    if ((prototype == nullptr) || GetWidgetClassPrototype(widgetClassName))
    {
        cxx_assert(false);
        return false;
    }
    mWidgetClassPrototypes[widgetClassName] = prototype;

    std::string typeName = typeid(*prototype).name();
    cxx_assert(mWidgetTypeToClassName.find(typeName) == mWidgetTypeToClassName.end());
    mWidgetTypeToClassName[typeName] = widgetClassName;
    return true;
}

UiWidget* UiWidgetManager::GetWidgetClassPrototype(const std::string& className) const
{
    auto proto_it = mWidgetClassPrototypes.find(className);
    if (proto_it != mWidgetClassPrototypes.end())
    {
        return proto_it->second;
    }
    return nullptr;
}
