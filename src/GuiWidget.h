#pragma once

#include "GuiDefs.h"

// basic gui element class
class GuiWidget: public cxx::handled_object<GuiWidget>
{
    friend class GuiManager;

public:
    std::string mName; // user-defined identifier for widget
    GuiUserData mUserData; // user-defined data

public:
    // construct widget
    GuiWidget(GuiWidgetClass* widgetClass);
    GuiWidget();
    virtual ~GuiWidget();

    // render widget and all children
    // @param renderContext: Gui render context
    void RenderFrame(GuiRenderer& renderContext);

    // process widget logic and all children
    // @param deltaTime: Time passed since previous update
    void UpdateFrame(float deltaTime);

protected:
    GuiWidgetClass* mClass; // cannot be null

    // layout params
    eGuiHorzAlignment mHorzAlign = eGuiHorzAlignment_None;
    eGuiVertAlignment mVertAlign = eGuiVertAlignment_None;
    eGuiOrigin mOrigin = eGuiOrigin_LeftTopCorner;
    Point2D mOriginPoint;
    GuiAnchors mAnchors;

    // initial location and dimensions, local space
    Point2D mStartPosition;
    Size2D mStartSize;
    Rect2D mStartClientRect; // position and size combined together

    // current location and dimensions, local space
    Point2D mPosition;
    Size2D mSize;
    Rect2D mClientRect; // position and size combined together




};

// base widget class
extern GuiWidgetClass gBaseWidgetClass;