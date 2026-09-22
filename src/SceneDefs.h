#pragma once

//////////////////////////////////////////////////////////////////////////

class Scene;
class SceneObject;
class EnvironmentMeshObject;
class AnimatingMeshObject;
class ProceduralMeshObject;
class RenderView;

//////////////////////////////////////////////////////////////////////////

enum eSceneObjectType
{
    eSceneObjectType_EnvironmentMesh,
    eSceneObjectType_AnimatingMesh,
    eSceneObjectType_ProceduralMesh,
    eSceneObjectType_COUNT
};

//////////////////////////////////////////////////////////////////////////

class ISceneObjectRenderer
{
public:
    virtual ~ISceneObjectRenderer()
    {
    }
    virtual void BeginFrame() = 0;
    virtual void EndFrame()  = 0;
    virtual void BeginBatch(Camera& camera) = 0;
    virtual void EndBatch() = 0;
    virtual void RenderInstance(eRenderPass currentPass, SceneObject* object) = 0;
};

//////////////////////////////////////////////////////////////////////////

enum eAnimationLoopMode
{
    eAnimationLoopMode_None, // doesn't loop
    eAnimationLoopMode_Repeat,
    eAnimationLoopMode_PingPong,    
};

struct AnimationParams
{
public:
    // frames range
    int mFirstFrame = 0;
    int mLastFrame = 0;
    // speed params
    float mFramesPerSecond = 30.0f;
    float mSpeedFactor = 1.0f;
    // loop params
    eAnimationLoopMode mLoopMode = eAnimationLoopMode_None;
};

//////////////////////////////////////////////////////////////////////////

struct SceneRenderLists
{
public:
    //////////////////////////////////////////////////////////////////////////
    struct Entry
    {
    public:
        SceneObject* mSceneObject {};
        eSceneObjectType mSceneObjectType {};
        float mDistanceToCamera2 = 0.0f; // squared
    };
    //////////////////////////////////////////////////////////////////////////
public:
    SceneRenderLists() = default;
    // reset lists
    inline void Clear()
    {
        for (int icouner = 0; icouner < eRenderPass_COUNT; ++icouner)
        {
            mListsPerPass[icouner].clear();
        }
    }
    // queue
    inline void Register(eRenderPass renderPass, SceneObject* object, eSceneObjectType objectType, float distanceToCamera2)
    {
        if (object)
        {
            Entry& objectEntry = mListsPerPass[renderPass].emplace_back();
            objectEntry.mSceneObject = object;
            objectEntry.mSceneObjectType = objectType;
            objectEntry.mDistanceToCamera2 = distanceToCamera2;
        }
    }
public:
    std::vector<Entry> mListsPerPass[eRenderPass_COUNT];
};

//////////////////////////////////////////////////////////////////////////