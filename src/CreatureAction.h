#pragma once

//////////////////////////////////////////////////////////////////////////

#include "CreatureDefs.h"

//////////////////////////////////////////////////////////////////////////

class CreatureAction: public cxx::noncopyable
{
public:

    //////////////////////////////////////////////////////////////////////////

    enum eStatus
    {
        eStatus_Init, // initial state
        eStatus_InProgress,
        eStatus_Completed,
    };

    enum eResult
    {
        eResult_None, // not completed yet
        eResult_Success,
        eResult_Failed,
        eResult_Cancelled
    };

    //////////////////////////////////////////////////////////////////////////

public:
    CreatureAction(eCreatureAction actionId);
    virtual ~CreatureAction();

    void Configure(Creature* creature);

    // pool
    virtual void OnRecycle();

    // status info
    inline eCreatureAction GetActionId() const { return mActionId; }
    inline bool IsAction(eCreatureAction actionId) const
    {
        return mActionId == actionId;
    }
    inline eStatus GetStatus() const { return mStatus; }
    inline eResult GetResult() const { return mResult; }
    // status shortcuts
    inline bool IsCompleted() const { return HasStatus(eStatus_Completed); }
    inline bool InProgress() const { return !HasStatus(eStatus_Completed); }
    inline bool HasStatus(eStatus status) const
    {
        return mStatus == status;
    }
    inline bool HasResult(eResult result) const 
    { 
        return HasStatus(eStatus_Completed) && (mResult == result); 
    }
    inline bool IsSuccess() const { return HasResult(eResult_Success); }
    inline bool IsFailed() const { return HasResult(eResult_Failed); }

    // soft interruption
    inline bool IsCancellationRequested() const { return mIsCancellationRequested; }
    void RequestCancellation();

    void EnterAction();
    void LeaveAction();
    void ResumeAction(eCreatureAction subActionId, eResult subActionResult);
    bool UpdateLogic(float stepDeltaTime);
    void ReceiveMsg(EntityMsg& msgData);

protected:
    void SetActionResult(eResult result);

    inline Creature* GetCreaturePtr() const { return mCreature; }
    inline Creature& GetCreature() const
    {
        cxx_assert(mCreature);
        return *mCreature;
    }

    // starts sub action, returns previous action
    CreatureActionPtr StartSubAction(CreatureActionPtr subAction);

    //////////////////////////////////////////////////////////////////////////
    // overridables
    virtual void HandleEnterAction() = 0;
    virtual void HandleLeaveAction() = 0;
    virtual void HandleUpdateLogic(float stepDeltaTime) = 0;
    virtual void HandleResumeAction(eCreatureAction subActionId, eResult subActionResult)
    {
    }
    virtual void HandleMessage(EntityMsg& msgData)
    {
    }
    //////////////////////////////////////////////////////////////////////////
private:
    eCreatureAction mActionId {};

    Creature* mCreature {};

    eStatus mStatus {};
    eResult mResult {};

    CreatureActionPtr mSubAction {};

    bool mIsCancellationRequested {};
};

//////////////////////////////////////////////////////////////////////////