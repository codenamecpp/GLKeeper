#include "stdafx.h"
#include "UiGridLayout.h"

UiGridLayout::UiGridLayout() : UiGridLayout("grid_layout")
{
    SetFillColor(0); // transparent
    SetBorderColor(0); // transparent
}

UiGridLayout::UiGridLayout(const std::string& widgetClassName) 
    : UiPanel(widgetClassName)
    , mOrientation(eUiLayoutOrientation_Horizontal)
    , mCols(1)
    , mRows(1)
    , mSpacingX()
    , mSpacingY()
{
    SetFillColor(0); // transparent
    SetBorderColor(0); // transparent
}

UiGridLayout::UiGridLayout(const UiGridLayout& sourceWidget)
    : UiPanel(sourceWidget)
    , mOrientation(sourceWidget.mOrientation)
    , mCols(sourceWidget.mCols)
    , mRows(sourceWidget.mRows)
    , mSpacingX(sourceWidget.mSpacingX)
    , mSpacingY(sourceWidget.mSpacingY)
{
}

UiGridLayout::~UiGridLayout()
{
}

void UiGridLayout::SetLayoutOrientation(eUiLayoutOrientation orientation)
{
    if (orientation == mOrientation)
        return;

    mOrientation = orientation;
    PerformLayout();
}

void UiGridLayout::Deserialize(const JsonElement& jsonElement)
{
    UiPanel::Deserialize(jsonElement);

    if (JsonElement orientationNode = jsonElement.FindElement("layout_orientation"))
    {
        const std::string orientation = orientationNode.GetValueString();
        bool isSuccess = cxx::parse_enum(orientation.c_str(), mOrientation);
        if (!isSuccess)
        {
            gConsole.LogMessage(eLogLevel_Warning, "Unknown layout orientation mode '%s'", orientation.c_str());
        }
        cxx_assert(isSuccess);
    }

    // setup number of rows and colunts
    if (JsonQuery(jsonElement, "num_cols", mCols) && mCols < 1)
    {
        mCols = 1;
    }
    
    if (JsonQuery(jsonElement, "num_rows", mRows) && mRows < 1)
    {
        mRows = 1;
    }

    // setup child spacing
    JsonQuery(jsonElement, "spacing_x", mSpacingX);
    JsonQuery(jsonElement, "spacing_y", mSpacingY);
}

UiGridLayout* UiGridLayout::CloneSelf() const
{
    UiGridLayout* clone = new UiGridLayout(*this);
    return clone;
}

void UiGridLayout::HandleChildAttached(UiWidget* widget)
{
    PerformLayout();
}

void UiGridLayout::HandleChildDetached(UiWidget* widget)
{
    PerformLayout();
}

void UiGridLayout::SetColsCount(int numCols)
{
    if (numCols < 1)
        numCols = 1;

    if (numCols == mCols)
        return;

    mCols = numCols;
    PerformLayout();
}

void UiGridLayout::SetRowsCount(int numRows)
{
    if (numRows < 1)
        numRows = 1;

    if (numRows == mRows)
        return;

    mRows = numRows;
    PerformLayout();
}

void UiGridLayout::SetElementsSpacing(int spacingX, int spacingY)
{
    if (mSpacingX == spacingX && mSpacingY == spacingY)
        return;

    mSpacingX = spacingX;
    mSpacingY = spacingY;
    PerformLayout();
}

void UiGridLayout::PerformLayout()
{
    int currX = 0;
    int currY = 0;
    int currIndex = 0;
    int currLineMaxElementSize = 0;

    cxx_assert(mOrientation < eUiLayoutOrientation_COUNT);
    const int maxElementsInLine = mOrientation == eUiLayoutOrientation_Horizontal ? mCols : mRows;
    cxx_assert(maxElementsInLine > 0);

    for (UiWidget* child = FirstChild(); child; child = child->NextSibling())
    {
        child->SetPosition({currX, currY});

        const Point2D& childSize = child->GetSize();
        if (mOrientation == eUiLayoutOrientation_Horizontal)
        {
            if (currLineMaxElementSize < childSize.y)
                currLineMaxElementSize = childSize.y;
            currX += childSize.x + mSpacingX;
        }
        else // vertical
        {
            if (currLineMaxElementSize < childSize.x)
                currLineMaxElementSize = childSize.x;
            currY += childSize.y + mSpacingY;
        }

        if (++currIndex == maxElementsInLine)
        {
            if (mOrientation == eUiLayoutOrientation_Horizontal)
            {
                currX = 0;
                currY += currLineMaxElementSize + mSpacingY;
            }
            else // vertical
            {
                currX += currLineMaxElementSize + mSpacingX;
                currY = 0;
            }
            currLineMaxElementSize = 0;
            currIndex = 0;
        }
    } // for
}
