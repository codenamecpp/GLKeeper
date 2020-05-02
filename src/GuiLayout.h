#pragma once

#include "GuiDefs.h"

// simple grid layout implementation
class GuiLayout: public cxx::noncopyable
{
public:
    GuiLayout(GuiWidget* container);
    ~GuiLayout();
    void CopyProperties(const GuiLayout& sourceLayout);

    // arrange widgets within container
    void LayoutElements();

    // read layout properties from json document
    void LoadProperties(cxx::json_node_object documentNode);

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

private:
    GuiWidget* mContainer;
    // grid layout params
    eGuiLayoutOrientation mOrientation;
    int mCols;
    int mRows;
    int mSpacingHorz;
    int mSpacingVert;
};