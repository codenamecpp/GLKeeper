#include "pch.h"
#include "GuiLayout.h"
#include "GuiWidget.h"

GuiLayout::GuiLayout(GuiWidget* container)
    : mContainer(container)
    , mOrientation()
    , mCols(1)
    , mRows(1)
    , mSpacingHorz()
    , mSpacingVert()
{
    debug_assert(mContainer);
}

GuiLayout::~GuiLayout()
{
}

void GuiLayout::CopyProperties(const GuiLayout& sourceLayout)
{
    mOrientation = sourceLayout.mOrientation;
    mCols = sourceLayout.mCols;
    mRows = sourceLayout.mRows;
    mSpacingHorz = sourceLayout.mSpacingHorz;
    mSpacingVert = sourceLayout.mSpacingVert;
}

void GuiLayout::LoadProperties(cxx::json_node_object documentNode)
{
    cxx::json_get_attribute(documentNode, "orientation", mOrientation);
    if (cxx::json_get_attribute(documentNode, "num_cols", mCols))
    {
        if (mCols < 1)
        {
            debug_assert(false);
            mCols = 1;
        }
    }
    if (cxx::json_get_attribute(documentNode, "num_rows", mRows))
    {
        if (mRows < 1)
        {
            debug_assert(false);
            mRows = 1;
        }
    }
    cxx::json_get_attribute(documentNode, "spacing_horz", mSpacingHorz);
    cxx::json_get_attribute(documentNode, "spacing_vert", mSpacingVert);
}

void GuiLayout::SetOrientation(eGuiLayoutOrientation orientation)
{
    if (orientation == mOrientation)
        return;

    mOrientation = orientation;
    LayoutElements();
}

void GuiLayout::SetColsCount(int numCols)
{
    if (numCols < 1)
        numCols = 1;

    if (numCols == mCols)
        return;

    mCols = numCols;
    LayoutElements();
}

void GuiLayout::SetRowsCount(int numRows)
{
    if (numRows < 1)
        numRows = 1;

    if (numRows == mRows)
        return;

    mRows = numRows;
    LayoutElements();
}

void GuiLayout::SetSpacing(int spacingHorz, int spacingVert)
{
    if (mSpacingHorz == spacingHorz && mSpacingVert == spacingVert)
        return;

    mSpacingHorz = spacingHorz;
    mSpacingVert = spacingVert;
    LayoutElements();
}

void GuiLayout::LayoutElements()
{
    if (mContainer == nullptr)
    {
        debug_assert(false);
        return;
    }

    Point currPos(0, 0);
    int currIndex = 0;
    int currLineMaxElementSize = 0;

    const int maxElementsInLine = mOrientation == eGuiLayoutOrientation_Horizontal ? mCols : mRows;
    debug_assert(maxElementsInLine > 0);

    for (GuiWidget* curr_child = mContainer->GetChild(); curr_child; 
        curr_child = curr_child->NextSibling())
    {
        curr_child->SetPosition(currPos);

        if (mOrientation == eGuiLayoutOrientation_Horizontal)
        {
            if (currLineMaxElementSize < curr_child->mSize.y)
                currLineMaxElementSize = curr_child->mSize.y;
            currPos.x += curr_child->mSize.x + mSpacingHorz;
        }
        else // vertical
        {
            if (currLineMaxElementSize < curr_child->mSize.x)
                currLineMaxElementSize = curr_child->mSize.x;
            currPos.y += curr_child->mSize.y + mSpacingVert;
        }

        if (++currIndex == maxElementsInLine)
        {
            if (mOrientation == eGuiLayoutOrientation_Horizontal)
            {
                currPos.x = 0;
                currPos.y += currLineMaxElementSize + mSpacingVert;
            }
            else // vertical
            {
                currPos.x += currLineMaxElementSize + mSpacingHorz;
                currPos.y = 0;
            }
            currLineMaxElementSize = 0;
            currIndex = 0;
        }
    } // for
}