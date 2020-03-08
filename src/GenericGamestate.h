#pragma once

#include "InputsDefs.h"

// game state controller interface
class GenericGamestate: public cxx::noncopyable
{
public:
    virtual ~GenericGamestate()
    {
    }

    // enter game state
    virtual void HandleGamestateEnter()
    {
    }

    // leave game state
    virtual void HandleGamestateLeave()
    {
    }

    // process game state frame logic
    // @param theDeltaTime: Time since previous frame
    virtual void HandleUpdateFrame() 
    {
    }

    // process input event
    // @param inputEvent: Event data
    virtual void HandleInputEvent(MouseButtonInputEvent& inputEvent)
    {
    }

    virtual void HandleInputEvent(MouseMovedInputEvent& inputEvent)
    {
    }

    virtual void HandleInputEvent(MouseScrollInputEvent& inputEvent)
    {
    }

    virtual void HandleInputEvent(KeyInputEvent& inputEvent)
    {
    }

    virtual void HandleInputEvent(KeyCharEvent& inputEvent)
    {
    }
};