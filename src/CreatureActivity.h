#pragma once

//////////////////////////////////////////////////////////////////////////

#include "CreatureDefs.h"

//////////////////////////////////////////////////////////////////////////

using CreatureActivity = std::variant<
    CreatureActivity_None,
    CreatureActivity_GoTo,
    CreatureActivity_GoToBed,
    CreatureActivity_GoToFood,
    CreatureActivity_Idle,
    CreatureActivity_Explore,
    CreatureActivity_Sleep,
    CreatureActivity_Eat
>;

//////////////////////////////////////////////////////////////////////////

class CreatureActivityBase
{
public:
    CreatureActivityBase(eCreatureActivity activityType)
        : mActivityType(activityType)
    {}
    inline eCreatureActivity GetType() const { return mActivityType; }
    inline eCreatureActivityStatus GetStatus() const { return mActivityStatus; }
    inline void SetStatus(eCreatureActivityStatus newStatus)
    {
        mActivityStatus = newStatus;
    }
    inline eCreatureActivityResult GetResult() const { return mActivityResult; }
    inline void SetResult(eCreatureActivityResult newResult)
    {
        if (newResult != eCreatureActivityResult_None)
        {
            mActivityStatus = eCreatureActivityStatus_Finished;
        }
        mActivityResult = newResult;
    }
private:
    eCreatureActivity mActivityType;
    eCreatureActivityStatus mActivityStatus = eCreatureActivityStatus_Init;
    eCreatureActivityResult mActivityResult = eCreatureActivityResult_None;
};

//////////////////////////////////////////////////////////////////////////

class CreatureActivity_None : public CreatureActivityBase 
{
public:
    CreatureActivity_None()
        : CreatureActivityBase(eCreatureActivity_None)
    {
    }
};

//////////////////////////////////////////////////////////////////////////

class CreatureActivity_GoTo: public CreatureActivityBase
{
public:
    CreatureActivity_GoTo(const glm::vec2& destinationPoint) 
        : CreatureActivityBase(eCreatureActivity_GoToLocation)
        , mDestinationPoint(destinationPoint)
    {
    }
    inline const glm::vec2& GetDestination() const { return mDestinationPoint; }
public:
    glm::vec2 mDestinationPoint;
};

//////////////////////////////////////////////////////////////////////////

class CreatureActivity_GoToBed: public CreatureActivityBase
{
public:
    CreatureActivity_GoToBed() 
        : CreatureActivityBase(eCreatureActivity_GoToBed)
    {
    }
};

//////////////////////////////////////////////////////////////////////////

class CreatureActivity_GoToFood: public CreatureActivityBase
{
public:
    CreatureActivity_GoToFood() 
        : CreatureActivityBase(eCreatureActivity_GoToFood)
    {
    }
};

//////////////////////////////////////////////////////////////////////////

class CreatureActivity_Idle: public CreatureActivityBase
{
public:
    CreatureActivity_Idle() 
        : CreatureActivityBase(eCreatureActivity_Idle)
    {
    }
public:
    cxx::uniqueptr<CreatureActivity> mGoToRandomPoint;
};

//////////////////////////////////////////////////////////////////////////

class CreatureActivity_Explore: public CreatureActivityBase
{
public:
    CreatureActivity_Explore() 
        : CreatureActivityBase(eCreatureActivity_Explore)
    {
    }
};

//////////////////////////////////////////////////////////////////////////

class CreatureActivity_Sleep: public CreatureActivityBase
{
public:
    CreatureActivity_Sleep() 
        : CreatureActivityBase(eCreatureActivity_Sleep)
    {

    }
};

//////////////////////////////////////////////////////////////////////////

class CreatureActivity_Eat: public CreatureActivityBase
{
public:
    CreatureActivity_Eat() 
        : CreatureActivityBase(eCreatureActivity_Eat)
    {
    }
};

//////////////////////////////////////////////////////////////////////////