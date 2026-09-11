#include "stdafx.h"
#include "UiWidgetManager.h"
#include "UiRenderContext.h"
#include "UiWidget.h"
#include "UiPicture.h"
#include "UiButton.h"
#include "UiView.h"
#include "UiPanel.h"
#include "UiTextBox.h"
#include "UiGridLayout.h"
#include "UiScrollBar.h"
#include "UiCompositeButton.h"

//////////////////////////////////////////////////////////////////////////

UiWidgetManager gWidgetManager;

//////////////////////////////////////////////////////////////////////////

bool UiWidgetManager::Initialize()
{
    RegisterWidgetClasses();

    return true;
}

void UiWidgetManager::Shutdown()
{
    mHoveredWidget = nullptr;
    mFocusedWidget = nullptr;

    mViewsToAttach.clear();
    mViewsToDetach.clear();
    mViews.clear();

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
    RegisterWidgetClass<UiTextBox>();
    RegisterWidgetClass<UiGridLayout>();
    RegisterWidgetClass<UiScrollBar>();
    RegisterWidgetClass<UiCompositeButton>();
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
    if (UiWidget* eventHandler = mFocusedWidget ? mFocusedWidget : mHoveredWidget)
    {
        eventHandler->InputEvent(inputEvent);
        inputEvent.SetConsumed(true);
    }

    // process interactive layers in reverse order
    IterateViews(true, [&inputEvent](UiView* uiview) -> bool
        {
            if (!inputEvent.mConsumed)
            {
                uiview->InputEvent(inputEvent);
            }
            return !inputEvent.mConsumed;
        });
}

void UiWidgetManager::InputEvent(MouseScrollInputEvent& inputEvent)
{
    // process focused
    if (UiWidget* eventHandler = mFocusedWidget)
    {
        mFocusedWidget->InputEvent(inputEvent);
    }

    // process hovered
    if (UiWidget* eventHandler = mHoveredWidget)
    {
        for (UiWidget* roller = eventHandler; roller; roller = roller->GetParent())
        {
            if (inputEvent.mConsumed)
                break;

            roller->InputEvent(inputEvent);
        }
    }

    // process interactive layers in reverse order
    IterateViews(true, [&inputEvent](UiView* uiview) -> bool
        {
            if (!inputEvent.mConsumed)
            {
                uiview->InputEvent(inputEvent);
            }
            return !inputEvent.mConsumed;
        });
}

void UiWidgetManager::InputEvent(MouseButtonInputEvent& inputEvent)
{
    if (UiWidget* eventHandler = mFocusedWidget ? mFocusedWidget : mHoveredWidget)
    {
        eventHandler->InputEvent(inputEvent);
        inputEvent.SetConsumed(true);
    }

    // process interactive layers in reverse order
    IterateViews(true, [&inputEvent](UiView* uiview) -> bool
        {
            if (!inputEvent.mConsumed)
            {
                uiview->InputEvent(inputEvent);
            }
            return !inputEvent.mConsumed;
        });
}

void UiWidgetManager::InputEvent(KeyCharEvent& inputEvent)
{
    if (UiWidget* eventHandler = mFocusedWidget ? mFocusedWidget : mHoveredWidget)
    {
        eventHandler->InputEvent(inputEvent);
        inputEvent.SetConsumed(true);
    }

    // process interactive layers in reverse order
    IterateViews(true, [&inputEvent](UiView* uiview) -> bool
        {
            if (!inputEvent.mConsumed)
            {
                uiview->InputEvent(inputEvent);
            }
            return !inputEvent.mConsumed;
        });
}

void UiWidgetManager::InputEvent(KeyInputEvent& inputEvent)
{
    if (UiWidget* eventHandler = mFocusedWidget ? mFocusedWidget : mHoveredWidget)
    {
        eventHandler->InputEvent(inputEvent);
        //inputEvent.SetConsumed(true);
    }

    // process interactive layers in reverse order
    IterateViews(true, [&inputEvent](UiView* uiview) -> bool
        {
            if (!inputEvent.mConsumed)
            {
                uiview->InputEvent(inputEvent);
            }
            return !inputEvent.mConsumed;
        });
}

void UiWidgetManager::RenderFrame(UiRenderContext& renderContext)
{
    // render interactive ui layers
    IterateViews(false, [&renderContext](UiView* uiview) -> bool
        {
            renderContext.SetTransform(nullptr);
            uiview->RenderFrame(renderContext);
            return true;
        });
}

void UiWidgetManager::UpdateFrame(float deltaTime)
{
    ProcessWidgetToDelete();
    ProcessViewListChanges();
    IterateViews(false, [deltaTime](UiView* uiview) -> bool
        {
            uiview->UpdateFrame(deltaTime);
            return true;
        });
    UpdateCurrentHovered();
    UpdateCurrentFocused();
}

void UiWidgetManager::UpdateCurrentHovered()
{
    UiWidget* currentHovered = nullptr;
    // process interactive layers in reverse order
    IterateViews(true, [&currentHovered](UiView* uiview) -> bool
        {
            UiWidget* hierarchyRoot = uiview->GetHierarchy().GetRootWidget();
            if (hierarchyRoot && hierarchyRoot->IsVisibleSelf())
            {
                currentHovered = hierarchyRoot->PickWidget(gInputs.GetMousePosition());
            }
            return (currentHovered == nullptr);
        });
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

void UiWidgetManager::ProcessViewListChanges()
{
    if (!mViewsToDetach.empty())
    {
        cxx::erase_elements(mViews, mViewsToDetach);

        mViewsToDetach.clear();
    }

    if (!mViewsToAttach.empty())
    {
        mViews.insert(mViews.end(), mViewsToAttach.begin(), mViewsToAttach.end());
        std::stable_sort(mViews.begin(), mViews.end(), [](UiView* lhs, UiView* rhs) 
            { 
                return lhs->mViewLayer < rhs->mViewLayer; 
            });
        mViewsToAttach.clear();
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
        prevHovered->MouseLeave();
    }

    if (mHoveredWidget)
    {
        mHoveredWidget->MouseEnter();
    }
}

UiWidget* UiWidgetManager::ConstructWidget(const std::string& className) const
{
    UiWidget* instance = nullptr;
    if (UiWidget* prototype = GetWidgetPrototype(className))
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
    mFocusedWidget->HandleFocusGain();
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
    if (view == nullptr)
        return;

    cxx::erase(mViewsToDetach, view);
    if (!cxx::contains(mViews, view))
    {
        mViewsToAttach.push_back(view);
    }
}

void UiWidgetManager::DetachView(UiView* view)
{
    cxx_assert(view);
    if (view == nullptr)
        return;

    cxx::erase(mViewsToAttach, view);
    if (cxx::contains(mViews, view))
    {
        mViewsToDetach.push_back(view);
    }
}

bool UiWidgetManager::HasViewAttached(const UiView* view) const
{
    return cxx::contains(mViewsToAttach, view) || 
        (cxx::contains(mViews, view) && !cxx::contains(mViewsToDetach, view));
}

void UiWidgetManager::ScreenSizeChanged(const Point2D& screenSize)
{
    ProcessViewListChanges();
    IterateViews(false, [&screenSize](UiView* uiview) -> bool
        {
            uiview->ScreenSizeChanged(screenSize);
            return true;
        });
}

bool UiWidgetManager::RegisterWidgetClass(const std::string& widgetClassName, UiWidget* prototype)
{
    if ((prototype == nullptr) || GetWidgetPrototype(widgetClassName))
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

UiWidget* UiWidgetManager::GetWidgetPrototype(const std::string& className) const
{
    auto proto_it = mWidgetClassPrototypes.find(className);
    if (proto_it != mWidgetClassPrototypes.end())
    {
        return proto_it->second;
    }
    return nullptr;
}


template<typename TProc>
void UiWidgetManager::IterateViews(bool inReverse, TProc proc)
{
    if (inReverse)
    {
        for (auto it = mViews.rbegin(); it != mViews.rend(); ++it)
        {
            UiView* currentView = *it;
            if (cxx::contains(mViewsToDetach, currentView))
                continue;

            if (!proc(currentView))
                break;
        }
    }
    else // forward
    {
        for (UiView* roller: mViews)
        {
            if (cxx::contains(mViewsToDetach, roller))
                continue;
            
            if (!proc(roller))
                break;
        }
    }
}
