#include "stdafx.h"
#include "UiCompositeButton.h"

//////////////////////////////////////////////////////////////////////////
// helpers

template<typename TProc>
inline void ParseTokens(std::string_view parseStr, TProc proc)
{
    size_t pos_start = parseStr.find_first_not_of(' ');
    while (pos_start != std::string_view::npos) 
    {
        size_t pos_end = parseStr.find_first_of(' ', pos_start);
        std::string_view token = (pos_end == std::string_view::npos) ?
            parseStr.substr(pos_start) :
            parseStr.substr(pos_start, pos_end - pos_start);

        proc(token);
        pos_start = parseStr.find_first_not_of(' ', pos_end);
    }
}

//////////////////////////////////////////////////////////////////////////

static const std::array<StringHash, eUiButtonState_COUNT> ButtonStateNamesHashList = []()
    {
        std::array<StringHash, eUiButtonState_COUNT> values {};
        for (int i = 0; i < eUiButtonState_COUNT; ++i)
        {
            values[i] = HashForString(cxx::enum_to_string(static_cast<eUiButtonState>(i)));
        }
        return values;
    }();

//////////////////////////////////////////////////////////////////////////

UiCompositeButton::UiCompositeButton() 
    : UiCompositeButton("cbutton")
{
}

UiCompositeButton::UiCompositeButton(const std::string& widgetClassName)
    : UiButton(widgetClassName)
{
}

UiCompositeButton::UiCompositeButton(const UiCompositeButton& sourceWidget)
    : UiButton(sourceWidget)
{
}

void UiCompositeButton::Deserialize(const JsonElement& jsonElement)
{
    UiButton::Deserialize(jsonElement);
}

void UiCompositeButton::SetCustomState(StringHash stateNameHash, bool isActive)
{
    bool stateChanged = false;
    if (isActive)
    {
        stateChanged = mActiveStatesHashSet.insert(stateNameHash).second;
    }
    else
    {
        stateChanged = mActiveStatesHashSet.erase(stateNameHash) > 0;
    }

    if (stateChanged)
    {
        RefreshCompositeElements();
    }
}

void UiCompositeButton::SetCustomState(std::string_view stateName, bool isActive)
{
    if (!stateName.empty())
    {
        SetCustomState(HashForString(stateName), isActive);
    }
}

bool UiCompositeButton::HasCustomState(StringHash stateNameHash) const
{
    return cxx::contains(mActiveStatesHashSet, stateNameHash);
}

bool UiCompositeButton::HasCustomState(std::string_view stateName) const
{
    return !stateName.empty() && HasCustomState(HashForString(stateName));
}

UiCompositeButton::~UiCompositeButton()
{
}

UiCompositeButton* UiCompositeButton::CloneSelf() const
{
    UiCompositeButton* clone = new UiCompositeButton(*this);
    return clone;
}

void UiCompositeButton::HandleChildAttached(UiWidget* widget)
{
    UiButton::HandleChildAttached(widget);
    RegisterElement(widget);
}

void UiCompositeButton::HandleChildDetached(UiWidget* widget)
{
    UiButton::HandleChildDetached(widget);
    UnregisterElement(widget);
}

void UiCompositeButton::ButtonStateChanged()
{
    UiButton::ButtonStateChanged();

    // update active button state
    StringHash currStateHash = ButtonStateNamesHashList[mButtonState];
    if (mLastButtonStateHash != currStateHash)
    {
        mActiveStatesHashSet.erase(mLastButtonStateHash);
        bool statesSetChanged = mActiveStatesHashSet.insert(currStateHash).second;
        cxx_assert(statesSetChanged);
        mLastButtonStateHash = currStateHash;
        if (statesSetChanged)
        {
            RefreshCompositeElements();
        }
    }
}

void UiCompositeButton::RefreshCompositeElements()
{
    for (const CompositeElement& roller: mElements)
    {
        RefreshElementState(roller);
    }
}

void UiCompositeButton::RegisterElement(UiWidget* widget)
{
    bool isExists = cxx::contains_if(mElements, [widget](const CompositeElement& roller)
        {
            return roller.mChildWidget == widget;
        });
    cxx_assert(!isExists);
    if (isExists)
        return;

    CompositeElement element {};
    element.mChildWidget = widget;

    // parse states string
    std::string statesStr;
    if (widget->CustomProps().GetProperty("show_states", statesStr))
    {
        ParseTokens(statesStr, [&element](std::string_view token)
            {
                StringHash stateNameHash = HashForString(token);
                auto statesSetChanged = element.mShowStates.insert(stateNameHash).second;
                cxx_assert(statesSetChanged);
            });
    }
    if (widget->CustomProps().GetProperty("hide_states", statesStr))
    {
        ParseTokens(statesStr, [&element](std::string_view token)
            {
                StringHash stateNameHash = HashForString(token);
                auto statesSetChanged = element.mHideStates.insert(stateNameHash).second;
                cxx_assert(statesSetChanged);
            });
    }

    if (element.mShowStates.empty() && element.mHideStates.empty())
        return;

    mElements.push_back(element);
    // force refresh
    RefreshElementState(mElements.back());
}

void UiCompositeButton::UnregisterElement(UiWidget* widget)
{
    cxx::erase_if(mElements, [widget](const CompositeElement& roller)
        {
            return roller.mChildWidget == widget;
        });
}

void UiCompositeButton::RefreshElementState(const CompositeElement& element) const
{
    bool isVisible = true;

    if (!element.mHideStates.empty())
    {
        isVisible = std::none_of(element.mHideStates.begin(), element.mHideStates.end(),
            [this](StringHash state) 
                { 
                    return cxx::contains(this->mActiveStatesHashSet, state); 
                }
            );
    }

    if (isVisible && !element.mShowStates.empty())
    {
        isVisible = std::any_of(element.mShowStates.begin(), element.mShowStates.end(),
            [this](StringHash state) 
                { 
                    return cxx::contains(this->mActiveStatesHashSet, state); 
                }
            );
    }

    element.mChildWidget->SetVisible(isVisible);
}

