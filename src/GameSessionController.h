#pragma once

//////////////////////////////////////////////////////////////////////////

class GameSessionController: public cxx::noncopyable
{
public:
    GameSessionController() = default;

    virtual ~GameSessionController();

    // overridables
    virtual void OnSessionLoaded();
    virtual void OnSessionStart();
    virtual void OnSessionShutdown();

    virtual void UpdateFrame(float deltaTime);
    virtual void UpdateLogic(float stepDeltaTime);
    virtual void InputEvent(MouseButtonInputEvent& inputEvent);
    virtual void InputEvent(KeyInputEvent& inputEvent);
    virtual void InputEvent(MouseMovedInputEvent& inputEvent);
    virtual void InputEvent(MouseScrollInputEvent& inputEvent);

private:

};

//////////////////////////////////////////////////////////////////////////