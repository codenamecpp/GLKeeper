#pragma once

#include "SceneDefs.h"
#include "SceneCamera.h"
#include "AABBTree.h"

// manages scene objects visualization and transformation
class GameScene: public cxx::noncopyable
{
public:
    SceneCamera mCamera; // main gamescene camera

public:

    // setup internal scene resources
    bool Initialize();
    void Deinit();

};

extern GameScene gGameScene;