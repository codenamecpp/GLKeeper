#pragma once

//////////////////////////////////////////////////////////////////////////

#include "CreatureActivity.h"

//////////////////////////////////////////////////////////////////////////

struct CreatureActivityUtils
{
public:

    // accessing common properties

    static eCreatureActivity GetType(CreatureActivity& activity)
    {
        eCreatureActivity resultType = eCreatureActivity_None;
        std::visit([&resultType](auto& a) { resultType = a.GetType(); }, activity);
        return resultType;
    }

    static eCreatureActivityStatus GetStatus(CreatureActivity& activity)
    {
        eCreatureActivityStatus resultStatus = eCreatureActivityStatus_Init;
        std::visit([&resultStatus](auto& a) { resultStatus = a.GetStatus(); }, activity);
        return resultStatus;
    }

    static void SetStatus(CreatureActivity& activity, eCreatureActivityStatus newStatus)
    {
        std::visit([newStatus](auto& a) { a.SetStatus(newStatus); }, activity);
    }

    static void SetCancellationStatus(CreatureActivity& activity)
    {
        if (GetStatus(activity) == eCreatureActivityStatus_Running)
        {
            SetStatus(activity, eCreatureActivityStatus_Cancelling);
        }
    }

    static eCreatureActivityResult GetResult(CreatureActivity& activity)
    {
        eCreatureActivityResult activityResult = eCreatureActivityResult_None;
        std::visit([&activityResult](auto& a) { activityResult = a.GetResult(); }, activity);
        return activityResult;
    }

    static void SetResult(CreatureActivity& activity, eCreatureActivityResult newResult)
    {
        std::visit([newResult](auto& a) { a.SetResult(newResult); }, activity);
    }

    static bool IsFinished(CreatureActivity& activity)
    {
        const eCreatureActivityStatus activityStatus = GetStatus(activity);
        return (activityStatus == eCreatureActivityStatus_Finished);
    }

    // construct activities

    template<typename TActivity, typename ... TArgs>
    static cxx::uniqueptr<CreatureActivity> Construct(TArgs&&... args)
    {
        cxx::uniqueptr<CreatureActivity> activity = Construct();
        if (activity)
        {
            activity->emplace<TActivity>(std::forward<TArgs>(args)...);
        }
        return std::move(activity);
    }

private:
    static cxx::uniqueptr<CreatureActivity> Construct();
};

//////////////////////////////////////////////////////////////////////////