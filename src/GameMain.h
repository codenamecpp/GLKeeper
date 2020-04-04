#pragma once

#include "MeshViewGamestate.h"
#include "GameplayGamestate.h"
#include "NullGamestate.h"
#include "StatisticsWindow.h"

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
    bool IsNullGamestate() const;

    void HandleScreenResolutionChanged();

private:
    void SwitchToGameState(GenericGamestate* gamestate);

private:
    MeshViewGamestate mMeshViewGamestate;
    GameplayGamestate mGameplayGamestate;
    NullGamestate mNullGamestate;

    StatisticsWindow mFpsWindow;
};

extern GameMain gGameMain;