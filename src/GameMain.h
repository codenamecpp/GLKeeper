#pragma once

#include "MeshViewGamestate.h"
#include "GameplayGamestate.h"
#include "LayoutsEditGamestate.h"
#include "ToolsUISceneStatisticsWindow.h"

// game core
class GameMain: public cxx::noncopyable
{
    // all gamestates can access private members
    friend class MeshViewGamestate;
    friend class GameplayGamestate;

public:
    // readonly
    GenericGamestate* mCurrentGamestate = nullptr;

public:

    // setup internal game resources and gamestates
    bool Initialize();
    void Deinit();

    // process single frame logic
    void UpdateFrame();

    // process debug draw
    void DebugRenderFrame(DebugRenderer& renderer);

    // process input event
    // @param inputEvent: Event data
    void HandleInputEvent(MouseButtonInputEvent& inputEvent);
    void HandleInputEvent(MouseMovedInputEvent& inputEvent);
    void HandleInputEvent(MouseScrollInputEvent& inputEvent);
    void HandleInputEvent(KeyInputEvent& inputEvent);
    void HandleInputEvent(KeyCharEvent& inputEvent);

    // determine current game state
    bool IsMeshViewGamestate() const;
    bool IsGameplayGamestate() const;
    bool IsLayoutsEditGamestate() const;

    void HandleScreenResolutionChanged();

private:
    void SwitchToGameState(GenericGamestate* gamestate);

private:
    MeshViewGamestate mMeshViewGamestate;
    GameplayGamestate mGameplayGamestate;
    LayoutsEditGamestate mLayoutsEditGamestate;

    ToolsUISceneStatisticsWindow mFpsWindow;
};

extern GameMain gGameMain;