#pragma once

//////////////////////////////////////////////////////////////////////////

#include "Animator.h"

//////////////////////////////////////////////////////////////////////////

struct CreatureAnimConst
{
    // animation states
    static const StringHash StatePose;
    static const StringHash StateWalk;
    static const StringHash StateIdleSelector;
    static const StringHash StateIdle1;
    static const StringHash StateIdle2;
    static const StringHash StateDigging;
    static const StringHash StateReinforceWall;
    static const StringHash StateClaimFloor;

    // animation parameters
    static const StringHash ParamIsWalk;
    static const StringHash ParamIsDigging;
    static const StringHash ParamIsReinforceWall;
    static const StringHash ParamIsClaimFloor;

    // helpers
    static bool IsIdleState(StringHash stateId)
    {
        return (stateId == StateIdleSelector) || (stateId == StateIdle1) || (stateId == StateIdle2);
    }
};

//////////////////////////////////////////////////////////////////////////