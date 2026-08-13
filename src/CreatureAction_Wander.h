#pragma once

//////////////////////////////////////////////////////////////////////////

#include "CreatureAction.h"

//////////////////////////////////////////////////////////////////////////

class CreatureAction_Wander: public CreatureAction
{
public:
    CreatureAction_Wander();

    void Configure(Creature* creature, const glm::vec2& destination);

    // override CreatureAction
    void OnRecycle() override;
    void HandleEnterAction() override;
    void HandleLeaveAction() override;
    void HandleUpdateLogic(float stepDeltaTime) override;
    void HandleResumeAction(eCreatureAction subActionId, eResult subActionResult) override;

private:
    glm::vec2 mDestination {};
};

//////////////////////////////////////////////////////////////////////////