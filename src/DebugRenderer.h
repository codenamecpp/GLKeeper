#pragma once

#include "GraphicsDefs.h"

// debug geometry visualization manager
class DebugRenderer: public cxx::noncopyable
{
public:

    // setup debug renderer internal resources
    // @returns false on error
    bool Initialize();
    void Shutdown();

private:

};

