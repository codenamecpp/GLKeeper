#pragma once

//////////////////////////////////////////////////////////////////////////

#include "CreatureAction.h"
#include "NavigationDefs.h"

//////////////////////////////////////////////////////////////////////////

class CreatureAction_WalkToPoint: public CreatureAction
{
public:
    CreatureAction_WalkToPoint();

    void Configure(Creature* creature, const glm::vec2& destination);

    // override CreatureAction
    void OnRecycle() override;
    void HandleEnterAction() override;
    void HandleLeaveAction() override;
    void HandleUpdateLogic(float stepDeltaTime) override;
    void HandleMessage(EntityMsg& msgData) override;

private:
    glm::vec2 mDestination {};
    PathFindRequestPtr mPathFindRequest;
};

//////////////////////////////////////////////////////////////////////////