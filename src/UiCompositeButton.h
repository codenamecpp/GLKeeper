#pragma once

//////////////////////////////////////////////////////////////////////////

#include "UiButton.h"

//////////////////////////////////////////////////////////////////////////

class UiCompositeButton: public UiButton
{
protected:
    //////////////////////////////////////////////////////////////////////////
    struct CompositeElement
    {
        UiWidget* mChildWidget {};
        std::unordered_set<StringHash> mShowStates {};
        std::unordered_set<StringHash> mHideStates {};
    };
    //////////////////////////////////////////////////////////////////////////
public:
    UiCompositeButton();

    // override UiButton
    void Deserialize(const JsonElement& jsonElement) override;

    // custom states
    void SetCustomState(StringHash stateNameHash, bool isActive);
    void SetCustomState(std::string_view stateName, bool isActive);
    bool HasCustomState(StringHash stateNameHash) const;
    bool HasCustomState(std::string_view stateName) const;

protected:
    UiCompositeButton(const std::string& widgetClassName);
    UiCompositeButton(const UiCompositeButton& sourceWidget); // clone props
    ~UiCompositeButton();

    UiCompositeButton* CloneSelf() const override;
    void HandleChildAttached(UiWidget* widget) override;
    void HandleChildDetached(UiWidget* widget) override;

    // override UiButton
    void ButtonStateChanged() override;

    void RefreshCompositeElements();

    void RegisterElement(UiWidget* widget);
    void UnregisterElement(UiWidget* widget);
    void RefreshElementState(const CompositeElement& element) const;

protected:
    std::vector<CompositeElement> mElements;
    StringHash mLastButtonStateHash {};
    std::unordered_set<StringHash> mActiveStatesHashSet;
};

//////////////////////////////////////////////////////////////////////////