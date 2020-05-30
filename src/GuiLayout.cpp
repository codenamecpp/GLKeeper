#include "pch.h"
#include "GuiLayout.h"
#include "GuiWidget.h"

GuiLayout::GuiLayout()
    : mOrientation(eGuiLayoutOrientation_Horizontal)
    , mCols(1)
    , mRows(1)
    , mSpacingHorz()
    , mSpacingVert()
    , mPaddingL()
    , mPaddingT()
    , mPaddingR()
    , mPaddingB()
    , mLayoutType(eGuiLayout_None)
{
}

void GuiLayout::LoadProperties(cxx::json_node_object documentNode)
{
    cxx::json_get_attribute(documentNode, "type", mLayoutType);
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

    if (cxx::json_node_object spacingNode = documentNode["spacing"])
    {
        cxx::json_get_attribute(spacingNode, "horz", mSpacingHorz);
        cxx::json_get_attribute(spacingNode, "vert", mSpacingVert);
    }

    if (cxx::json_node_object paddingNode = documentNode["padding"])
    {
        cxx::json_get_attribute(paddingNode, "left", mPaddingL);
        cxx::json_get_attribute(paddingNode, "top", mPaddingT);
        cxx::json_get_attribute(paddingNode, "bottom", mPaddingB);
        cxx::json_get_attribute(paddingNode, "right", mPaddingR);
    }
}

void GuiLayout::Clear()
{
    mLayoutType = eGuiLayout_None;
    mOrientation = eGuiLayoutOrientation_Horizontal;
    mCols = 1;
    mRows = 1;
    mSpacingHorz = 0;
    mSpacingVert = 0;
    mPaddingL = 0;
    mPaddingT = 0;
    mPaddingR = 0;
    mPaddingB = 0;
}

void GuiLayout::SetOrientation(eGuiLayoutOrientation orientation)
{
    if (orientation == mOrientation)
        return;

    mOrientation = orientation;
}

void GuiLayout::SetColsCount(int numCols)
{
    if (numCols < 1)
        numCols = 1;

    if (numCols == mCols)
        return;

    mCols = numCols;
}

void GuiLayout::SetRowsCount(int numRows)
{
    if (numRows < 1)
        numRows = 1;

    if (numRows == mRows)
        return;

    mRows = numRows;
}

void GuiLayout::SetSpacing(int spacingHorz, int spacingVert)
{
    if (mSpacingHorz == spacingHorz && mSpacingVert == spacingVert)
        return;

    mSpacingHorz = spacingHorz;
    mSpacingVert = spacingVert;
}

void GuiLayout::SetPadding(int paddingL, int paddingT, int paddingR, int paddingB)
{
    if (mPaddingB == paddingB && 
        mPaddingL == paddingL && 
        mPaddingR == paddingR && 
        mPaddingT == paddingT)
    {
        return;
    }
    mPaddingB = paddingB;
    mPaddingL = paddingL;
    mPaddingR = paddingR; 
    mPaddingT = paddingT;
}

void GuiLayout::LayoutSimpleGrid(GuiWidget* container)
{
    Point currPos(mPaddingL, mPaddingT);
    int currIndex = 0;
    int currLineMaxElementSize = 0;

    const int maxElementsInLine = mOrientation == eGuiLayoutOrientation_Horizontal ? mCols : mRows;
    debug_assert(maxElementsInLine > 0);

    for (GuiWidget* curr_child = container->GetChild(); curr_child; 
        curr_child = curr_child->NextSibling())
    {
        if (!curr_child->IsVisible())
            continue;

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
                currPos.x = mPaddingL;
                currPos.y += currLineMaxElementSize + mSpacingVert;
            }
            else // vertical
            {
                currPos.x += currLineMaxElementSize + mSpacingHorz;
                currPos.y = mPaddingT;
            }
            currLineMaxElementSize = 0;
            currIndex = 0;
        }
    } // for
}

void GuiLayout::LayoutElements(GuiWidget* container)
{
    if (container == nullptr)
    {
        debug_assert(false);
        return;
    }

    switch (mLayoutType)
    {
        case eGuiLayout_SimpleGrid:
            LayoutSimpleGrid(container);    
        break;
    }
}