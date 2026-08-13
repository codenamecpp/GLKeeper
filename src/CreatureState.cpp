#include "stdafx.h"
#include "CreatureState.h"
#include "CreatureAction.h"
#include "CreatureTask.h"

CreatureState::CreatureState(eCreatureState stateId)
    : mStateId(stateId)
{
}

CreatureState::~CreatureState()
{
}

void CreatureState::EnterState(eCreatureState prevState)
{
    cxx_assert(mCreature);
    cxx_assert(mStateDuration == 0.0f);
    HandleEnterState(prevState);
}

void CreatureState::LeaveState(eCreatureState nextState)
{
    StartStateAction(nullptr);
    HandleLeaveState(nextState);
}

void CreatureState::UpdateLogic(float stepDeltaTime)
{
    mStateDuration += stepDeltaTime;
    cxx_assert(mStateDuration >= 0.0f);
    // process child actions
    if (mStateAction && mStateAction->InProgress())
    {
        mStateAction->UpdateLogic(stepDeltaTime);
    }
    HandleUpdateLogic(stepDeltaTime);
}

void CreatureState::ReceiveMsg(EntityMsg& msgData)
{
    // process child actions
    if (mStateAction && mStateAction->InProgress())
    {
        mStateAction->ReceiveMsg(msgData);
    }
    HandleMessage(msgData);
}

CreatureActionPtr CreatureState::StartStateAction(CreatureActionPtr action)
{
    CreatureActionPtr tempAction;
    if (mStateAction)
    {
        mStateAction.swap(tempAction);
    }

    if (tempAction)
    {
        tempAction->LeaveAction();
    }

    // sanity check
    cxx_assert(mStateAction == nullptr);
    if (mStateAction == nullptr)
    {
        mStateAction = std::move(action);
        if (mStateAction)
        {
            mStateAction->EnterAction();
        }
    }
    return std::move(tempAction);
}

void CreatureState::OnRecycle()
{
    mCreature = nullptr;
    mStateDuration = {};

    cxx_assert(mStateAction == nullptr);
    mStateAction.reset();
}

void CreatureState::Configure(Creature* creature)
{
    mCreature = creature;
}
