#pragma once

#include "UiPanel.h"

//////////////////////////////////////////////////////////////////////////
// Simple grid layout panel
//////////////////////////////////////////////////////////////////////////

class UiGridLayout: public UiPanel
{
public:
    UiGridLayout();

    // Arrange child widgets
    virtual void PerformLayout();

    // Set orientation mode for child widgets layout
    void SetLayoutOrientation(eUiLayoutOrientation orientation);

    // Setup number of grid rows/columns
    void SetColsCount(int numCols);
    void SetRowsCount(int numRows);

    // Get number of grid rows/columns
    inline void GetColsRowsCount(int& numCols, int& numRows) const
    {
        numCols = mCols;
        numRows = mRows;
    }

    // Set minimum distance between child elements by x and y coords
    void SetElementsSpacing(int spacingX, int spacingY);

    // Get minimum distance between child elements by x and y coords
    inline void GetElementsSpacing(int& spacingX, int& spacingY) const
    {
        spacingX = mSpacingX;
        spacingY = mSpacingY;
    }

    // Load widget properties from json
    void Deserialize(const JsonElement& jsonElement) override;

protected:
    UiGridLayout(const std::string& widgetClassName);
    UiGridLayout(const UiGridLayout& sourceWidget); // clone props
    ~UiGridLayout();

    UiGridLayout* CloneSelf() const override;

    // override UiWidget
    void HandleChildAttached(UiWidget* widget) override;
    void HandleChildDetached(UiWidget* widget) override;

protected:
    eUiLayoutOrientation mOrientation;
    int mCols;
    int mRows;
    int mSpacingX;
    int mSpacingY;
};
