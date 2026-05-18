#pragma once

//////////////////////////////////////////////////////////////////////////

#include "DebugRenderer.h"

//////////////////////////////////////////////////////////////////////////

class IDebugVisualizer: public cxx::noncopyable
{
public:
    virtual ~IDebugVisualizer()
    {}
    // @param theDebugRenderer: Debug renderer instance
    virtual void OnDebugDraw(DebugRenderer& theDebugRenderer) = 0;
};