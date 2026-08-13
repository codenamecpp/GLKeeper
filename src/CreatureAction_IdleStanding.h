#pragma once

//////////////////////////////////////////////////////////////////////////

#include "CreatureAction.h"

//////////////////////////////////////////////////////////////////////////

class CreatureAction_IdleStanding: public CreatureAction
{
public:
    CreatureAction_IdleStanding();

    // override CreatureAction
    void HandleEnterAction() override;
    void HandleLeaveAction() override;
    void HandleUpdateLogic(float stepDeltaTime) override;

private:
};

//////////////////////////////////////////////////////////////////////////