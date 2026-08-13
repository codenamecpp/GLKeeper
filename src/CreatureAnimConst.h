#pragma once

//////////////////////////////////////////////////////////////////////////

#include "Animator.h"

//////////////////////////////////////////////////////////////////////////

struct CreatureAnimConst
{
    // animation states
    static const Animator::NameHash StatePose;
    static const Animator::NameHash StateWalk;
    static const Animator::NameHash StateIdleSelector;
    static const Animator::NameHash StateIdle1;
    static const Animator::NameHash StateIdle2;
    static const Animator::NameHash StateDigging;
    static const Animator::NameHash StateReinforceWall;
    static const Animator::NameHash StateClaimFloor;

    // animation parameters
    static const Animator::NameHash ParamIsWalk;
    static const Animator::NameHash ParamIsDigging;
    static const Animator::NameHash ParamIsReinforceWall;
    static const Animator::NameHash ParamIsClaimFloor;

    // helpers
    static bool IsIdleState(Animator::NameHash stateId)
    {
        return (stateId == StateIdleSelector) || (stateId == StateIdle1) || (stateId == StateIdle2);
    }
};

//////////////////////////////////////////////////////////////////////////