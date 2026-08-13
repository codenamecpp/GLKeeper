#include "stdafx.h"
#include "CreatureAnimConst.h"

//////////////////////////////////////////////////////////////////////////

const Animator::NameHash CreatureAnimConst::StatePose = Animator::NameToHash("pose_frame");
const Animator::NameHash CreatureAnimConst::StateWalk = Animator::NameToHash("walk");
const Animator::NameHash CreatureAnimConst::StateIdleSelector = Animator::NameToHash("idle_selector");
const Animator::NameHash CreatureAnimConst::StateIdle1 = Animator::NameToHash("idle_1");
const Animator::NameHash CreatureAnimConst::StateIdle2 = Animator::NameToHash("idle_2");
const Animator::NameHash CreatureAnimConst::StateDigging = Animator::NameToHash("digging");
const Animator::NameHash CreatureAnimConst::StateReinforceWall = Animator::NameToHash("reinforce_wall");
const Animator::NameHash CreatureAnimConst::StateClaimFloor = Animator::NameHash("claim_floor");

//////////////////////////////////////////////////////////////////////////

const Animator::NameHash CreatureAnimConst::ParamIsWalk = Animator::NameToHash("is_walk");
const Animator::NameHash CreatureAnimConst::ParamIsDigging = Animator::NameToHash("is_digging");
const Animator::NameHash CreatureAnimConst::ParamIsReinforceWall = Animator::NameToHash("is_reinforce_wall");
const Animator::NameHash CreatureAnimConst::ParamIsClaimFloor = Animator::NameToHash("is_claim_floor");

//////////////////////////////////////////////////////////////////////////