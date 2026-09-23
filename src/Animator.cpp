#include "stdafx.h"
#include "Animator.h"
#include "AnimatingMeshObject.h"
#include "MeshAssetManager.h"
#include "WeightSelector.h"

Animator::Animator()
{
}

void Animator::Configure(AnimatingMeshObject* animatingMesh)
{
    mAnimatingMesh = animatingMesh;
    cxx_assert(mAnimatingMesh);
}

void Animator::Clear()
{
    mAnimatingMesh = nullptr;

    mAnyStateTransitions.clear();
    mStartStateTransitions.clear();

    mStates.clear();
    mParameters.Clear();
    mCurrentState = nullptr;
    mCurrentStatePhase = {};

    mTransitionRequest.reset();
}

void Animator::UpdateFrame(float deltaTime)
{
    cxx_assert(mAnimatingMesh);
    if (mAnimatingMesh && mAnimatingMesh->IsObjectActive())
    {
        PreUpdateState();
        UpdateState();
        PostUpdateState();
    }
}

void Animator::ResetTrigger(StringHash paramId)
{
    auto param_it = mParameters.mTriggers.find(paramId);
    if (param_it == mParameters.mTriggers.find(paramId))
    {
        cxx_assert(false);
        return;
    }
    param_it->second = false;
}

void Animator::SetTrigger(StringHash paramId)
{
    auto param_it = mParameters.mTriggers.find(paramId);
    if (param_it == mParameters.mTriggers.find(paramId))
    {
        cxx_assert(false);
        return;
    }
    param_it->second = true;
}

void Animator::SetParamValue(StringHash paramId, bool value)
{
    auto param_it = mParameters.mBooleans.find(paramId);
    if (param_it == mParameters.mBooleans.end())
    {
        cxx_assert(false);
        return;
    }
    param_it->second = value;
}

void Animator::SetParamValue(StringHash paramId, float value)
{
    auto param_it = mParameters.mFloats.find(paramId);
    if (param_it == mParameters.mFloats.end())
    {
        cxx_assert(false);
        return;
    }
    param_it->second = value;
}

bool Animator::CheckCondition(const Condition& condition) const
{
    bool conditionResult = false;
    switch (condition.mConditionType)
    {
        case eConditionType_False:
        {
            auto it = mParameters.mBooleans.find(condition.mParamId);
            if (it != mParameters.mBooleans.end())
            {
                conditionResult = !it->second;
            }
        }
        break;

        case eConditionType_True:
        {
            auto it = mParameters.mBooleans.find(condition.mParamId);
            if (it != mParameters.mBooleans.end())
            {
                conditionResult = it->second;
            }
        }
        break;

        case eConditionType_GreaterThan:
        {
            auto it = mParameters.mFloats.find(condition.mParamId);
            if (it != mParameters.mFloats.end())
            {
                conditionResult = it->second > condition.mFloatValue;
            }
        }
        break;

        case eConditionType_LessThan:
        {
            auto it = mParameters.mFloats.find(condition.mParamId);
            if (it != mParameters.mFloats.end())
            {
                conditionResult = it->second < condition.mFloatValue;
            }
        }
        break;

        case eConditionType_Trigger:
        {
            auto it = mParameters.mTriggers.find(condition.mParamId);
            if (it != mParameters.mTriggers.end())
            {
                conditionResult = it->second;
            }
        }
        break;

        default:
            cxx_assert(false);
        break;
    }
    return conditionResult;
}

bool Animator::CheckTransitionConditions(const ConditionalTransition& transition) const
{    
    // always fails if no conditions
    if (transition.mConditions.empty())
    {
        cxx_assert(false);
        return false;
    }
    for (const Condition& roller: transition.mConditions)
    {
        if (!CheckCondition(roller)) return false;
    }
    return true;
}

bool Animator::SelectFirstBestTransition(const ConditionalTransitions& transitions, eTransitionRequestOrigin origin)
{
    int transitionIndex = cxx::get_first_index_if(transitions, 
        [this](const ConditionalTransition& transition)
        {
            return this->CheckTransitionConditions(transition);
        });

    if (transitionIndex == -1) return false;

    bool canRequest = true;

    // try interrupt current transition request
    if (IsTransitionRequested())
    {
        const TransitionRequest& currentRequest = *mTransitionRequest;

        if (
            // cannot interrupt transition of higher priority with same origin
            ((currentRequest.mRequestOrigin == origin) && (currentRequest.mTransitionPriority < transitionIndex)) 
            ||
            // cannot interrupt transition from any-state
            (currentRequest.mRequestOrigin == eTransitionRequestOrigin_AnyState))
        {
            canRequest = false;
        }
    }

    if (canRequest)
    {
        const ConditionalTransition& targetTransition = transitions[transitionIndex];
        ConsumeTriggers(targetTransition);
        // skip transition to same state
        if (targetTransition.mTargetState != mCurrentState)
        {
            RequestTransition(targetTransition, origin, transitionIndex);
        }
    }

    return true;
}

bool Animator::SelectFirstBestTransition(const SimpleTransitions& transitions)
{
    int transitionIndex = 0;

    if (transitions.empty()) return false;
    if (transitions.size() > 1)
    {
        static WeightSelector<int> transitionSelector;
        transitionSelector.Clear();

        int transitionsCounter = 0;
        for (const SimpleTransition& roller: transitions)
        {
            transitionSelector.AddSelectionElement(transitionsCounter++, roller.mSelectionWeight);
        }
        cxx_assert(!transitionSelector.IsEmpty()); // sanity check
        transitionIndex = transitionSelector.SelectRandomElement();
    }
    RequestTransition(transitions[transitionIndex], {}, {});
    return true;
}

void Animator::PreUpdateState()
{
    // not started yet
    if (mCurrentState == nullptr) return;

    // not started yet
    if (mCurrentState == nullptr) return;

    // no active transition case
    if (!IsTransitionRequested())
    {
        bool selected = 
            SelectFirstBestTransition(mAnyStateTransitions, eTransitionRequestOrigin_AnyState) ||
            SelectFirstBestTransition(mCurrentState->mConditionalTransitions, eTransitionRequestOrigin_CurrentState);
    }
    // active transition interruption case
    else
    {
        const eTransitionInterruptionSource interruptionSrc = mTransitionRequest->mTransition.mInterruptionSource;
        
        // cannot be interrupted
        if (interruptionSrc == eTransitionInterruptionSource_None)
            return;

        // check any-state first
        if (SelectFirstBestTransition(mAnyStateTransitions, eTransitionRequestOrigin_AnyState))
            return;

        State* targetState = mTransitionRequest->mTransition.mTargetState;
        switch (interruptionSrc)
        {
            case eTransitionInterruptionSource_CurrentState:
            {
                bool selected = SelectFirstBestTransition(mCurrentState->mConditionalTransitions, eTransitionRequestOrigin_CurrentState);
            }
            break;

            case eTransitionInterruptionSource_NextState:
            {
                bool selected = SelectFirstBestTransition(targetState->mConditionalTransitions, eTransitionRequestOrigin_NextState);
            }
            break;

            case eTransitionInterruptionSource_CurrentStateThenNextState:
            {
                bool selected = 
                    SelectFirstBestTransition(mCurrentState->mConditionalTransitions, eTransitionRequestOrigin_CurrentState) ||
                    SelectFirstBestTransition(targetState->mConditionalTransitions, eTransitionRequestOrigin_NextState);
            }
            break;

            case eTransitionInterruptionSource_NextStateThenCurrentState:
            {
                bool selected = 
                    SelectFirstBestTransition(targetState->mConditionalTransitions, eTransitionRequestOrigin_NextState) ||
                    SelectFirstBestTransition(mCurrentState->mConditionalTransitions, eTransitionRequestOrigin_CurrentState);
            }
            break;

            default:
                cxx_assert(false);
            break;
        }
    }
}

void Animator::UpdateState()
{
    // not started yet
    if (mCurrentState == nullptr) return;

    // phase: start clip
    if (mCurrentStatePhase == eStatePhase_ClipStart)
    {
        if (!mAnimatingMesh->HasAnimation() || mAnimatingMesh->HasAnimationCompleted())
        {
            ChangePhase(IsTransitionRequested() ? eStatePhase_ClipEnd : eStatePhase_ClipMain);
        }
    }

    // phase: main clip
    if (mCurrentStatePhase == eStatePhase_ClipMain)
    {
#if 1 // in case if transition requested: wait current cycle finish
        if (IsTransitionRequested())
        {
            mAnimatingMesh->BreakAnimationLooping();
        }

        if (!mAnimatingMesh->HasAnimation() || mAnimatingMesh->HasAnimationCompleted())
        {
            ChangePhase(eStatePhase_ClipEnd);
        }
#else // interrupt immediately
        if (!mAnimatingMesh->HasAnimation() || mAnimatingMesh->HasAnimationCompleted() || IsTransitionRequested())
        {
            ChangePhase(eStatePhase_ClipEnd);
        }
#endif
    }

    // phase: end clip
    if (mCurrentStatePhase == eStatePhase_ClipEnd)
    {
        if (!mAnimatingMesh->HasAnimation() || mAnimatingMesh->HasAnimationCompleted())
        {
            ChangePhase(eStatePhase_Stop);
        }
    }

    // phase: stop
    if (mCurrentStatePhase == eStatePhase_Stop)
    {
        // do nothing
    }
}

void Animator::ChangePhase(eStatePhase nextPhase)
{
    cxx_assert(mCurrentState);

    // force finish current animation
    mAnimatingMesh->BreakAnimationLooping();
    mAnimatingMesh->RewindToEnd();

    bool hasStartClip = mCurrentState->mClipStart.has_value();
    bool hasEndClip = mCurrentState->mClipEnd.has_value();

    if (nextPhase == eStatePhase_ClipStart)
    {
        if (hasStartClip)
        {
            ChangeAnimationClip(*mCurrentState->mClipStart);
        }
        else // force next valid phase
        {
            nextPhase = eStatePhase_ClipMain;
        }
    }

    if (nextPhase == eStatePhase_ClipMain)
    {
        ChangeAnimationClip(mCurrentState->mClipMain);
    }

    if (nextPhase == eStatePhase_ClipEnd)
    {
        if (hasEndClip)
        {
            ChangeAnimationClip(*mCurrentState->mClipEnd);
        }
        else // force next valid phase
        {
            nextPhase = eStatePhase_Stop;
        }
    }

    if (nextPhase == eStatePhase_Stop)
    {
        // force last valid phase animation
        ChangeAnimationClip(hasEndClip ? *mCurrentState->mClipEnd : mCurrentState->mClipMain);

        mAnimatingMesh->BreakAnimationLooping();
        mAnimatingMesh->RewindToEnd();
    }

    mCurrentStatePhase = nextPhase;
}

void Animator::ChangeState(State* targetState, eStatePhase statePhase)
{
    cxx_assert(targetState);

    if (targetState == nullptr) return;

    mCurrentState = targetState;

    ChangePhase(statePhase);
    UpdateState();
}

void Animator::ChangeState(StringHash stateId, eStatePhase statePhase)
{
    State* targetState = GetState(stateId);
    ChangeState(targetState, statePhase);
    // discard transition request
    mTransitionRequest.reset();
}

void Animator::ChangeAnimationClip(const Clip& animationClip)
{
    cxx_assert(mAnimatingMesh);
    if (animationClip.mAssetName.empty())
    {
        cxx_assert(false);
        return;
    }
    MeshAsset* meshAsset = gMeshAssetManager.GetMesh(animationClip.mAssetName);
    cxx_assert(meshAsset);
    if (animationClip.mAnimParams)
    {
        mAnimatingMesh->Configure(meshAsset, *animationClip.mAnimParams);
    }
    else
    {
        mAnimatingMesh->Configure(meshAsset);
        mAnimatingMesh->ResetAnimationSpeedFactor();
    }
}

Animator::State* Animator::GetState(StringHash stateId)
{
    auto states_it = mStates.find(stateId);
    if (states_it != mStates.end())
    {
        return &states_it->second;
    }
    return nullptr;
}

void Animator::Start()
{
    cxx_assert(mCurrentState == nullptr);
    if (mCurrentState) return;

    if (SelectFirstBestTransition(mStartStateTransitions))
    {
        ExecuteTransition();
    }

    cxx_assert(mCurrentState);
}

void Animator::Restart()
{
    mCurrentState = nullptr;
    mCurrentStatePhase = {};
    mTransitionRequest.reset();
    Start();
}

Animator& Animator::DefineState(StringHash stateId, const ArtResourceDefinition& resourceDefinition, std::optional<eAnimationLoopMode> overrideLoopMode)
{
    cxx_assert(stateId != 0);

    if (stateId && (GetState(stateId) == nullptr))
    {
        // anim mesh
        if (resourceDefinition.mResourceType == eArtResource_AnimatingMesh)
        {
            State& state = mStates[stateId];
            state.mId = stateId;
            // main
            {
                Clip& animationClip = state.mClipMain;
                animationClip.mAssetName = resourceDefinition.mResourceName;

                MeshAsset* meshAsset = gMeshAssetManager.GetMesh(resourceDefinition.mResourceName);

                AnimationParams& animParams = animationClip.mAnimParams.emplace();
                animParams.mFramesPerSecond = resourceDefinition.mAnimationDesc.mFps * 1.0f;
                animParams.mFirstFrame = 0;
                animParams.mLastFrame = (meshAsset->GetAnimFramesCount() > 0) ? (meshAsset->GetAnimFramesCount() - 1) : 0;

                if (overrideLoopMode.has_value())
                {
                    animParams.mLoopMode = *overrideLoopMode;
                }
                else if (!resourceDefinition.mDoesntLoop)
                {
                    switch (meshAsset->GetAnimFrameFactorFunc())
                    {
                        case MeshAsset::eAnimFrameFactorFunc::Clamp: 
                            animParams.mLoopMode = eAnimationLoopMode_PingPong;
                        break;
                        case MeshAsset::eAnimFrameFactorFunc::Wrap: 
                            animParams.mLoopMode = eAnimationLoopMode_Repeat;
                        break;
                        default:
                            cxx_assert(false);
                            animParams.mLoopMode = eAnimationLoopMode_Repeat;
                        break;
                    }
                }
            }
            // start
            if (resourceDefinition.mHasStartAnimation)
            {
                Clip& animationClip = state.mClipStart.emplace();
                animationClip.mAssetName = resourceDefinition.mResourceName + "start";

                MeshAsset* meshAsset = gMeshAssetManager.GetMesh(animationClip.mAssetName);

                AnimationParams& animParams = animationClip.mAnimParams.emplace();
                animParams.mFramesPerSecond = resourceDefinition.mAnimationDesc.mFps * 1.0f;
                animParams.mFirstFrame = 0;
                animParams.mLastFrame = (meshAsset->GetAnimFramesCount() > 0) ? (meshAsset->GetAnimFramesCount() - 1) : 0;
            }
            // end
            if (resourceDefinition.mHasEndAnimation)
            {
                Clip& animationClip = state.mClipEnd.emplace();
                animationClip.mAssetName = resourceDefinition.mResourceName + "end";

                MeshAsset* meshAsset = gMeshAssetManager.GetMesh(animationClip.mAssetName);

                AnimationParams& animParams = animationClip.mAnimParams.emplace();
                animParams.mFramesPerSecond = resourceDefinition.mAnimationDesc.mFps * 1.0f;
                animParams.mFirstFrame = 0;
                animParams.mLastFrame = (meshAsset->GetAnimFramesCount() > 0) ? (meshAsset->GetAnimFramesCount() - 1) : 0;
            }
        }
        // static mesh
        else if ((resourceDefinition.mResourceType == eArtResource_Mesh) ||
            (resourceDefinition.mResourceType == eArtResource_TerrainMesh))
        {
            State& state = mStates[stateId];
            state.mId = stateId;
            // main
            {
                Clip& animationClip = state.mClipMain;
                animationClip.mAssetName = resourceDefinition.mResourceName;

                AnimationParams& animParams = animationClip.mAnimParams.emplace();
                animParams.mFramesPerSecond = 4.0f; // 1/4 of second
                animParams.mFirstFrame = 0;
                animParams.mLastFrame = 0;
                animParams.mLoopMode = overrideLoopMode.has_value() ? *overrideLoopMode : eAnimationLoopMode_Repeat;
            }
        }
        else
        {
            cxx_assert(false);
        }
    }
    else
    {
        cxx_assert(false);
    }
    return *this;
}

Animator& Animator::DefineTransition(StringHash fromStateId, StringHash toStateId, std::vector<Condition>&& conditions, 
    eTransitionMode transitionMode,
    eTransitionInterruptionSource interruptionSource)
{
    cxx_assert((toStateId > 0) && (fromStateId > 0));
    cxx_assert(!conditions.empty());

    State* originState = GetState(fromStateId);
    State* targetState = GetState(toStateId);

    cxx_assert(originState && targetState);

    if (originState && targetState)
    {
        ConditionalTransition& transition = originState->mConditionalTransitions.emplace_back();
            transition.mTargetState = targetState;
            transition.mTransitionMode = transitionMode;
            transition.mConditions = conditions;
            transition.mInterruptionSource = interruptionSource;
        CollectConditionParams(transition.mConditions);
    }
    return *this;
}

Animator& Animator::DefineTransition(StringHash fromStateId, StringHash toStateId, float selectionWeight)
{
    cxx_assert((toStateId > 0) && (fromStateId > 0));

    State* originState = GetState(fromStateId);
    State* targetState = GetState(toStateId);

    cxx_assert(originState && targetState);

    if (originState && targetState)
    {
        SimpleTransition& transition = originState->mSimpleTransitions.emplace_back();
            transition.mTargetState = targetState;
            transition.mSelectionWeight = selectionWeight;
        cxx_assert(selectionWeight >= 0.0f);
    }
    return *this;
}

Animator& Animator::DefineTransitionFromStart(StringHash toStateId, float selectionWeight)
{
    cxx_assert(toStateId > 0);

    State* targetState = GetState(toStateId);
    cxx_assert(targetState);

    if (targetState)
    {
        SimpleTransition& transition = mStartStateTransitions.emplace_back();
            transition.mTargetState = targetState;
            transition.mSelectionWeight = selectionWeight;
        cxx_assert(selectionWeight >= 0.0f);
    }
    return *this;
}

Animator& Animator::DefineTransitionFromAnyState(StringHash toStateId, std::vector<Condition>&& conditions, 
    eTransitionMode transitionMode,
    eTransitionInterruptionSource interruptionSource)
{
    cxx_assert(toStateId > 0);
    cxx_assert(!conditions.empty());

    State* targetState = GetState(toStateId);
    cxx_assert(targetState);

    if (targetState)
    {
        ConditionalTransition& transition = mAnyStateTransitions.emplace_back();
            transition.mTargetState = targetState;
            transition.mTransitionMode = transitionMode;
            transition.mConditions = conditions;
            transition.mInterruptionSource = interruptionSource;
        CollectConditionParams(transition.mConditions);
    }
    return *this;
}

void Animator::CollectConditionParams(const std::vector<Condition>& conditions)
{
    for (const Condition& roller: conditions)
    {
        if (roller.mParamId == 0)
        {
            cxx_assert(false);
            continue;
        }

        switch (roller.mConditionType)
        {
            case eConditionType_False:
            case eConditionType_True:
                mParameters.mBooleans.emplace(roller.mParamId, false);
            break;

            case eConditionType_GreaterThan:
            case eConditionType_LessThan:
                mParameters.mFloats.emplace(roller.mParamId, 0.0f);
            break;

            case eConditionType_Trigger:
                mParameters.mTriggers.emplace(roller.mParamId, false);
            break;

            default:
                cxx_assert(false);
            break;
        }
    }
}

void Animator::PostUpdateState()
{
    if (mCurrentState == nullptr) return;

    if (IsCurrentStateStopped())
    {
        if (!IsTransitionRequested())
        {
            SelectFirstBestTransition(mCurrentState->mSimpleTransitions);
        }
        ExecuteTransition();
        return;
    }

    // current state is in progress
    if (IsTransitionRequested())
    {
        // force next state if immediate mode
        const TransitionRequest& request = *mTransitionRequest;
        if (request.mTransition.mTransitionMode == eTransitionMode_Immediate)
        {
            ExecuteTransition();
        }
    }
}

void Animator::SetAnimSpeedFactor(StringHash stateId, float speedFactor)
{
    if (State* state = GetState(stateId))
    {
        // main clip
        if (state->mClipMain.mAnimParams.has_value())
        {
            state->mClipMain.mAnimParams->mSpeedFactor = speedFactor;
        }
        // start clip
        if (state->mClipStart.has_value() && 
            state->mClipStart->mAnimParams.has_value())
        {
            state->mClipStart->mAnimParams->mSpeedFactor = speedFactor;
        }
        // end clip
        if (state->mClipEnd.has_value() &&
            state->mClipEnd->mAnimParams.has_value())
        {
            state->mClipEnd->mAnimParams->mSpeedFactor = speedFactor;
        }

        // apply to current animation
        if ((mCurrentState == state) && mAnimatingMesh)
        {
            mAnimatingMesh->SetAnimationSpeedFactor(speedFactor);
        }
    }
    else
    {
        cxx_assert(false);
    }
}

void Animator::SetAnimSpeedFactor(StringHash stateId, eStatePhase statePhase, float speedFactor)
{
    if (statePhase == eStatePhase_Stop) 
    {
        cxx_assert(false);
        return;
    }

    if (State* state = GetState(stateId))
    {
        switch (statePhase)
        {
            case eStatePhase_ClipStart:
            {
                if (state->mClipStart.has_value() && 
                    state->mClipStart->mAnimParams.has_value())
                {
                    state->mClipStart->mAnimParams->mSpeedFactor = speedFactor;
                }
            }
            break;
            case eStatePhase_ClipMain:
            {
                if (state->mClipMain.mAnimParams.has_value())
                {
                    state->mClipMain.mAnimParams->mSpeedFactor = speedFactor;
                }
            }
            break;
            case eStatePhase_ClipEnd:
            {
                if (state->mClipEnd.has_value() &&
                    state->mClipEnd->mAnimParams.has_value())
                {
                    state->mClipEnd->mAnimParams->mSpeedFactor = speedFactor;
                }
            }
            break;
            default:
                cxx_assert(false);
            break;
        }
        // apply to current animation
        if ((mCurrentState == state) && (mCurrentStatePhase == statePhase) && mAnimatingMesh)
        {
            mAnimatingMesh->SetAnimationSpeedFactor(speedFactor);
        }
    }
    else
    {
        cxx_assert(false);
    }
}

void Animator::ResetAnimSpeedFactor(StringHash stateId)
{
    SetAnimSpeedFactor(stateId, 1.0f);
}

void Animator::ResetAnimSpeedFactor(StringHash stateId, eStatePhase statePhase)
{
    SetAnimSpeedFactor(stateId, statePhase, 1.0f);
}

void Animator::ConsumeTriggers(const ConditionalTransition& transition)
{
    for (const Condition& roller: transition.mConditions)
    {
        if (roller.mConditionType != eConditionType_Trigger)
            continue;

        ResetTrigger(roller.mParamId);
    }
}

void Animator::RequestTransition(const Transition& transition, eTransitionRequestOrigin origin, int priority)
{
    cxx_assert(transition.mTargetState);

    TransitionRequest& request = mTransitionRequest.emplace();
    {
        request.mTransition = transition;
        request.mRequestOrigin = origin;
        request.mTransitionPriority = priority;
    }
}

bool Animator::ExecuteTransition()
{
    bool isTransitionRequested = IsTransitionRequested();

    if (isTransitionRequested)
    {
        State* targetState = mTransitionRequest->mTransition.mTargetState;
        mTransitionRequest.reset();
        ChangeState(targetState);
    }
    return isTransitionRequested;
}

bool Animator::HasState(StringHash stateId) const
{
    return mStates.find(stateId) != mStates.end();
}
