#pragma once

//////////////////////////////////////////////////////////////////////////

#include "CreatureState.h"

//////////////////////////////////////////////////////////////////////////

class CreatureState_InHand: public CreatureState
{
public:
    CreatureState_InHand();

    // override CreatureState
    void HandleEnterState(eCreatureState prevState) override;
    void HandleLeaveState(eCreatureState nextState) override;
    void HandleUpdateLogic(float stepDeltaTime) override;
private:
};

//////////////////////////////////////////////////////////////////////////
