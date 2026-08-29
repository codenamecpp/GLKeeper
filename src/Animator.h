#pragma once

//////////////////////////////////////////////////////////////////////////

#include "SceneDefs.h"

//////////////////////////////////////////////////////////////////////////

class Animator final: public cxx::noncopyable
{
public:

    //////////////////////////////////////////////////////////////////////////

    using NameHash = size_t;

    static NameHash NameToHash(std::string_view name);

    //////////////////////////////////////////////////////////////////////////

    enum eStatePhase
    {
        eStatePhase_ClipStart,
        eStatePhase_ClipMain,
        eStatePhase_ClipEnd,
        eStatePhase_Stop,
    };

    // condition used for state transitions

    enum eConditionType
    {
        eConditionType_True, // bool
        eConditionType_False, // bool
        eConditionType_LessThan, // float
        eConditionType_GreaterThan, // float
        eConditionType_Trigger,
    };

    // conditional transition mode
    enum eTransitionMode
    {
        // breaks looping, waits animation to finish
        eTransitionMode_WaitAnimation,

        // instant change state
        eTransitionMode_Immediate,
    };

    enum eTransitionInterruptionSource
    {
        // cannot interrupt current transition
        eTransitionInterruptionSource_None, 

        // transitions from the current state (or any state) with the highest priority can interrupt the current transition
        eTransitionInterruptionSource_CurrentState, 

        // transitions from the next state (or any state) with the highest priority can interrupt the current transition
        eTransitionInterruptionSource_NextState,

        eTransitionInterruptionSource_CurrentStateThenNextState,
        eTransitionInterruptionSource_NextStateThenCurrentState,
    };

    struct Condition
    {
    public:
        eConditionType mConditionType = eConditionType_True;
        NameHash mParamId;

        // comparsion value used for eConditionType_LessThen, eConditionType_GreaterThen
        // has no meaning for eConditionType_True/eConditionType_False/eConditionType_Trigger
        float mFloatValue = {};
    };

    //////////////////////////////////////////////////////////////////////////

private:

    //////////////////////////////////////////////////////////////////////////
    struct State;
    struct Clip
    {
    public:
        std::string mAssetName;
        std::optional<AnimationParams> mAnimParams;
    };

    struct Transition
    {
    public:
        State* mTargetState = nullptr;
        eTransitionMode mTransitionMode {};
        eTransitionInterruptionSource mInterruptionSource {};
    };

    enum eTransitionRequestOrigin
    {
        eTransitionRequestOrigin_AnyState,
        eTransitionRequestOrigin_CurrentState,
        eTransitionRequestOrigin_NextState,
    };

    struct TransitionRequest
    {
    public:
        Transition mTransition {};
        eTransitionRequestOrigin mRequestOrigin {};

        // priority is index within origin transitions list, 0 is highest
        int mTransitionPriority = 0;
    };

    // conditional transition
    // it is triggers when all conditions evaluate to true (logical AND)
    // fails if conditions list empty
    struct ConditionalTransition: public Transition
    {
    public:
        std::vector<Condition> mConditions;
    };

    // simple (unconditional) transition 
    // it is triggered after the previous state finishes
    // if multiple transitions are available, one is selected randomly based on its selection weight
    struct SimpleTransition: public Transition
    {
    public:
        float mSelectionWeight = 50.0f;
    };

    using ConditionalTransitions = std::vector<ConditionalTransition>;
    using SimpleTransitions = std::vector<SimpleTransition>;
  
    struct State
    {
    public:
        NameHash mId {};

        // animation clips
        std::optional<Clip> mClipStart;
        Clip mClipMain;
        std::optional<Clip> mClipEnd;

        // state transition lists
        ConditionalTransitions mConditionalTransitions;
        SimpleTransitions mSimpleTransitions;
    };

    struct Parameters
    {
    public:
        Parameters() = default;
        inline void Clear()
        {
            mBooleans.clear();
            mFloats.clear();
            mTriggers.clear();
        }
    public:
        std::unordered_map<NameHash, bool> mBooleans;
        std::unordered_map<NameHash, float> mFloats;
        std::unordered_map<NameHash, bool> mTriggers;
    };

    //////////////////////////////////////////////////////////////////////////

public:
    Animator();
    void Configure(AnimatingMeshObject* animatingMesh);
    void Clear();

    // process update
    void UpdateFrame(float deltaTime);

    //////////////////////////////////////////////////////////////////////////
    // setup states and transitions
    //////////////////////////////////////////////////////////////////////////

    Animator& DefineState(NameHash stateId, const ArtResourceDefinition& resourceDefinition, 
        std::optional<eAnimationLoopMode> overrideLoopMode = std::nullopt);

    Animator& DefineTransition(NameHash fromStateId, NameHash toStateId, std::vector<Condition>&& conditions, 
        eTransitionMode transitionMode,
        eTransitionInterruptionSource interruptionSource);
    Animator& DefineTransition(NameHash fromStateId, NameHash toStateId, float selectionWeight);
    Animator& DefineTransitionFromStart(NameHash toStateId, float selectionWeight);
    Animator& DefineTransitionFromAnyState(NameHash toStateId, std::vector<Condition>&& conditions,
        eTransitionMode transitionMode,
        eTransitionInterruptionSource interruptionSource);

    //////////////////////////////////////////////////////////////////////////
    // control
    //////////////////////////////////////////////////////////////////////////

    void Start();
    void Restart();

    // forcible change current state or current state phase
    void ChangeState(NameHash stateId, eStatePhase statePhase = eStatePhase_ClipStart);
    void ChangePhase(eStatePhase nextPhase);

    void SetAnimSpeedFactor(NameHash stateId, float speedFactor);
    void SetAnimSpeedFactor(NameHash stateId, eStatePhase statePhase, float speedFactor);

    void ResetAnimSpeedFactor(NameHash stateId);
    void ResetAnimSpeedFactor(NameHash stateId, eStatePhase statePhase);

    //////////////////////////////////////////////////////////////////////////
    // parameters
    //////////////////////////////////////////////////////////////////////////

    void ResetTrigger(NameHash paramId);
    void SetTrigger(NameHash paramId);
    void SetParamValue(NameHash paramId, bool value);
    void SetParamValue(NameHash paramId, float value);

    //////////////////////////////////////////////////////////////////////////
    // status
    //////////////////////////////////////////////////////////////////////////

    inline NameHash GetCurrentState() const { return mCurrentState ? mCurrentState->mId : 0; }

    inline bool IsCurrentState(NameHash stateId) const 
    { 
        return (stateId == (mCurrentState ? mCurrentState->mId : 0));
    }

    inline bool IsCurrentPhase(eStatePhase statePhase) const
    {
        return mCurrentStatePhase == statePhase;
    }

    inline bool IsCurrentStateStopped() const { return IsCurrentPhase(eStatePhase_Stop); }
    inline bool IsStateStopped(NameHash stateId) const 
    { 
        return IsCurrentState(stateId) && IsCurrentPhase(eStatePhase_Stop); 
    }

    bool HasState(NameHash stateId) const;

    //////////////////////////////////////////////////////////////////////////
    // helpers
    //////////////////////////////////////////////////////////////////////////

    static Condition ConditionForBool(NameHash paramId, bool paramValue)
    {
        Condition condition { paramValue ? eConditionType_True : eConditionType_False };
            condition.mParamId = paramId;
        return condition;
    }

    static Condition ConditionForLessThan(NameHash paramId, float paramValue)
    {
        Condition condition { eConditionType_LessThan };
            condition.mParamId = paramId;
            condition.mFloatValue = paramValue;
        return condition;
    }

    static Condition ConditionForGreaterThan(NameHash paramId, float paramValue)
    {
        Condition condition { eConditionType_GreaterThan };
            condition.mParamId = paramId;
            condition.mFloatValue = paramValue;
        return condition;
    }

    static Condition ConditionForTrigger(NameHash triggerId)
    {
        Condition condition { eConditionType_Trigger };
            condition.mParamId = triggerId;
        return condition;
    }

private:
    bool CheckTransitionConditions(const ConditionalTransition& transition) const;
    bool CheckCondition(const Condition& condition) const;

    bool SelectFirstBestTransition(const ConditionalTransitions& transitions, eTransitionRequestOrigin origin);
    bool SelectFirstBestTransition(const SimpleTransitions& transitions);

    void ChangeAnimationClip(const Clip& animationClip);
    void UpdateState();
    void PreUpdateState();
    void PostUpdateState();
    void CollectConditionParams(const std::vector<Condition>& conditions);
    void ConsumeTriggers(const ConditionalTransition& transition);

    State* GetState(NameHash stateId);

    void ChangeState(State* targetState, eStatePhase statePhase = eStatePhase_ClipStart);

    void RequestTransition(const Transition& transition, eTransitionRequestOrigin origin, int priority);
    bool ExecuteTransition();
    bool IsTransitionRequested() const
    {
        return mTransitionRequest.has_value();
    }
    
private:
    AnimatingMeshObject* mAnimatingMesh = nullptr;

    std::unordered_map<NameHash, State> mStates;

    ConditionalTransitions mAnyStateTransitions;
    SimpleTransitions mStartStateTransitions;

    Parameters mParameters;

    // current state
    State* mCurrentState = nullptr;
    eStatePhase mCurrentStatePhase {};
   
    std::optional<TransitionRequest> mTransitionRequest;
};

//////////////////////////////////////////////////////////////////////////