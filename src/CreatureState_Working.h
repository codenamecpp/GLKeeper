#pragma once

//////////////////////////////////////////////////////////////////////////

#include "CreatureState.h"

//////////////////////////////////////////////////////////////////////////

class CreatureState_Working: public CreatureState
{
public:
    CreatureState_Working();

    // override CreatureState
    void HandleEnterState(eCreatureState prevState) override;
    void HandleLeaveState(eCreatureState nextState) override;
    void HandleUpdateLogic(float stepDeltaTime) override;

private:
    bool StartActionForTask(CreatureTask* creatureTask);
};

//////////////////////////////////////////////////////////////////////////