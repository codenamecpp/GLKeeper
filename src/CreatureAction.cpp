#include "stdafx.h"
#include "CreatureAction.h"

CreatureAction::CreatureAction(eCreatureAction actionId)
    : mActionId(actionId)
{
}

CreatureAction::~CreatureAction()
{
    cxx_assert(mSubAction == nullptr);
}

void CreatureAction::Configure(Creature* creature)
{
    mCreature = creature;
    cxx_assert(mCreature);
}

void CreatureAction::RequestCancellation()
{
    if (IsCompleted() || IsCancellationRequested())
        return;

    mIsCancellationRequested = true;
    // propagate cancellation request to child actions
    if (mSubAction)
    {
        mSubAction->RequestCancellation();
    }
}

void CreatureAction::OnRecycle()
{
    cxx_assert(!HasStatus(eStatus_InProgress));
    mCreature = nullptr;
    mStatus = {};
    mResult = {};
    mIsCancellationRequested = false;
    cxx_assert(mSubAction == nullptr);
    mSubAction.reset();
}

void CreatureAction::EnterAction()
{
    cxx_assert(HasStatus(eStatus_Init));
    if (HasStatus(eStatus_Init))
    {
        mStatus = eStatus_InProgress;
        HandleEnterAction();
    }
}

void CreatureAction::LeaveAction()
{
    StartSubAction(nullptr);

    if (HasStatus(eStatus_Init))
    {
        SetActionResult(eResult_Failed);
        return;
    }

    if (HasStatus(eStatus_InProgress))
    {
        SetActionResult(eResult_Failed);
    }

    HandleLeaveAction();
    cxx_assert(mSubAction == nullptr);
}

void CreatureAction::ResumeAction(eCreatureAction actionId, eResult actionResult)
{
    cxx_assert(HasStatus(eStatus_InProgress));
    if (HasStatus(eStatus_InProgress))
    {
        cxx_assert(mSubAction == nullptr);
        HandleResumeAction(actionId, actionResult);   
    }
}

bool CreatureAction::UpdateLogic(float stepDeltaTime)
{
    if (HasStatus(eStatus_InProgress))
    {
        if (mSubAction)
        {
            // update sub action
            if (!mSubAction->UpdateLogic(stepDeltaTime))
            {
                CreatureActionPtr subActionPtr = StartSubAction(nullptr);
                cxx_assert(subActionPtr);
                HandleResumeAction(subActionPtr->GetActionId(), subActionPtr->GetResult());
            }
            // skip self update
        }
        else
        {
            HandleUpdateLogic(stepDeltaTime);
        }
    }
    return HasStatus(eStatus_InProgress);
}

void CreatureAction::ReceiveMsg(EntityMsg& msgData)
{
    if (!HasStatus(eStatus_InProgress))
        return;

    if (mSubAction)
    {
        mSubAction->ReceiveMsg(msgData);
    }
    else
    {
        HandleMessage(msgData);
    }
}

void CreatureAction::SetActionResult(eResult result)
{
    cxx_assert(InProgress());
    mStatus = eStatus_Completed;
    mResult = result;
    cxx_assert(mResult != eResult_None);
}

CreatureActionPtr CreatureAction::StartSubAction(CreatureActionPtr subAction)
{
    if (subAction)
    {
        cxx_assert(InProgress());
    }

    CreatureActionPtr tempSubAction;
    if (mSubAction)
    {
        mSubAction.swap(tempSubAction);
    }

    if (tempSubAction)
    {
        tempSubAction->LeaveAction();
    }

    // sanity check
    cxx_assert(mSubAction == nullptr);
    if (mSubAction == nullptr)
    {
        mSubAction = std::move(subAction);
        if (mSubAction)
        {
            mSubAction->EnterAction();
        }
    }

    return std::move(tempSubAction);
}
