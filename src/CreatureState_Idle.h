#pragma once

//////////////////////////////////////////////////////////////////////////

#include "CreatureState.h"

//////////////////////////////////////////////////////////////////////////

class CreatureState_Idle: public CreatureState
{
public:
    CreatureState_Idle();

    // override CreatureState
    void HandleEnterState(eCreatureState prevState) override;
    void HandleLeaveState(eCreatureState nextState) override;
    void HandleUpdateLogic(float stepDeltaTime) override;
};

//////////////////////////////////////////////////////////////////////////