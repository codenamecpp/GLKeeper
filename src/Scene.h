#pragma once

//////////////////////////////////////////////////////////////////////////

#include "SceneDefs.h"
#include "Camera.h"
#include "AABBTree.h"
#include "IDebugVisualizer.h"

//////////////////////////////////////////////////////////////////////////

class Scene: private IDebugVisualizer
{
private:
    friend class SceneObject;
    friend class Animator;

public:
    Scene() = default;

    // one time init/deinit
    void Initialize();
    void Shutdown();

    void ClearScene();
    void UpdateFrame(float deltaTime);

    // accessing scene main camera
    inline Camera& GetCamera() { return mCamera; }

    inline const Camera& GetCamera() const { return mCamera; }

    // cast ray in specific viewport coordinate using current camera
    bool CastRayFromScreenPoint(const Point2D& screenCoordinate, cxx::ray3d_t& resultRay);

    // render lists querying
    void CollectObjectsForRender(SceneRenderLists& renderList);
    void CollectObjectsForRender(Camera& camera, SceneRenderLists& renderList);
    
    bool QueryObjects(const cxx::ray3d_t& ray, cxx::any_vector<SceneObject*> queryResult);
    bool QueryObjects(const cxx::ray3d_t& ray, Camera& camera, cxx::any_vector<SceneObject*> queryResult);

    // create scene objects

    // note: 
    //  newly created objects are initially inactive and will not be drawn or updated
    //  to activate an object, call object->SetActive(true)

    // the object gets destroyed when its uniqueptr is reset

    cxx::uniqueptr<EnvironmentMeshObject> CreateLavaMesh(cxx::span<MapTile*> mapTiles);
    cxx::uniqueptr<EnvironmentMeshObject> CreateWaterMesh(cxx::span<MapTile*> mapTiles);
    cxx::uniqueptr<AnimatingMeshObject> CreateAnimatingMesh();
    cxx::uniqueptr<ProceduralMeshObject> CreateProceduralMesh();

    // stats
    inline int GetActiveSceneObjectCount() const 
    { 
        return static_cast<int>(mActiveObjects.size()); 
    }

private:
    // override IDebugVisualizer
    void OnDebugDraw(DebugRenderer& theDebugRenderer) override;

    // handle object events
    void OnAnimatorBecomeActive(SceneObject* object);
    void OnAnimatorBecomeInactive(SceneObject* object);
    void OnObjectInvalidateTransform(SceneObject* object);
    void OnObjectInvalidateBounds(SceneObject* object);

    // aabbtree
    void BuildObjectsAABBTree();

private:
    Camera mCamera;

    AABBTree mAABBTree;

    std::vector<SceneObject*> mActiveObjects;
    std::vector<SceneObject*> mChangedObjects;
};

//////////////////////////////////////////////////////////////////////////

extern Scene gScene;

//////////////////////////////////////////////////////////////////////////

