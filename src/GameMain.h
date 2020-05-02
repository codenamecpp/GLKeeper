#pragma once

#include "MeshViewGamestate.h"
#include "GameplayGamestate.h"
#include "GuiTestGamestate.h"
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
    void ProcessInputEvent(MouseButtonInputEvent& inputEvent);
    void ProcessInputEvent(MouseMovedInputEvent& inputEvent);
    void ProcessInputEvent(MouseScrollInputEvent& inputEvent);
    void ProcessInputEvent(KeyInputEvent& inputEvent);
    void ProcessInputEvent(KeyCharEvent& inputEvent);

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
    GuiTestGamestate mLayoutsEditGamestate;

    ToolsUISceneStatisticsWindow mFpsWindow;
};

extern GameMain gGameMain;