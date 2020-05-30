#pragma once

#include "GuiDefs.h"

// layout implementation
class GuiLayout
{
public:
    GuiLayout();

    // arrange widgets within container
    void LayoutElements(GuiWidget* container);

    // read layout properties from json document
    void LoadProperties(cxx::json_node_object documentNode);

    void Clear();

    // set orientation mode for elements
    // @param orientation: Orientation mode
    void SetOrientation(eGuiLayoutOrientation orientation);

    // setup number of grid rows/columns
    // @param numCols: Number of columns, affected only if layout orientation is horizontal
    // @param numRows: Number of rows, affected only if layout orientation is vertical
    void SetColsCount(int numCols);
    void SetRowsCount(int numRows);

    // set minimum distance between child elements by x and y coords
    // @param spacingHorz, spacingVert: Distance
    void SetSpacing(int spacingHorz, int spacingVert);

    // padding will reduce the inside of the container element without changing it‘s size
    void SetPadding(int paddingL, int paddingT, int paddingR, int paddingB);

private:
    void LayoutSimpleGrid(GuiWidget* container);

private:
    eGuiLayout mLayoutType;
    eGuiLayoutOrientation mOrientation;

    int mCols;
    int mRows;

    int mSpacingHorz;
    int mSpacingVert;
    
    int mPaddingL; // left padding
    int mPaddingT; // top padding
    int mPaddingR; // right padding
    int mPaddingB; // bottom padding
};