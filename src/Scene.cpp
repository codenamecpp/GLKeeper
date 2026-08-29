#include "stdafx.h"
#include "Scene.h"
#include "SimplePool.h"
#include "EnvironmentMeshObject.h"
#include "AnimatingMeshObject.h"
#include "ProceduralMeshObject.h"
#include "GameRenderManager.h"
#include "Animator.h"

//////////////////////////////////////////////////////////////////////////

// pools

static SimplePool<EnvironmentMeshObject> gEnvironmentMeshObjectsPool = (
    [](EnvironmentMeshObject* object)
    {
        object->SetObjectActive(false);
        object->OnRecycle();
    });

//////////////////////////////////////////////////////////////////////////

Scene gScene;

//////////////////////////////////////////////////////////////////////////

void Scene::UpdateFrame(float deltaTime)
{
    // process objects
    for (SceneObject* object: mActiveObjects)
    {
        object->UpdateFrame(deltaTime);
    }

    BuildObjectsAABBTree();
}

void Scene::Initialize()
{
    gGameRenderer.RegisterDebugVisualizer(this);
}

void Scene::Shutdown()
{
    ClearScene();
    gGameRenderer.UnRegisterDebugVisualizer(this);
}

void Scene::ClearScene()
{
    // make sure to destroy all active objects
    mChangedObjects.clear();

    cxx_assert(mActiveObjects.empty());

    mAABBTree.Cleanup();
}

bool Scene::CastRayFromScreenPoint(const Point2D& screenCoordinate, cxx::ray3d_t& resultRay)
{
    const Viewport& viewport = gRenderDevice.GetViewport();

    Camera& sceneCamera = GetCamera();
    sceneCamera.ComputeMatricesAndFrustum(viewport);

    // wrap y
    const int mouseY = viewport.mScreenArea.h - screenCoordinate.y;

    glm::ivec4 vp ( viewport.mScreenArea.x, viewport.mScreenArea.y, viewport.mScreenArea.w, viewport.mScreenArea.h );
    //unproject twice to build a ray from near to far plane
    const glm::vec3 v0 = glm::unProject(glm::vec3{screenCoordinate.x * 1.0f, mouseY * 1.0f, 0.0f}, 
        sceneCamera.mViewMatrix, 
        sceneCamera.mProjectionMatrix, vp); // near plane

    const glm::vec3 v1 = glm::unProject(glm::vec3{screenCoordinate.x * 1.0f, mouseY * 1.0f, 1.0f}, 
        sceneCamera.mViewMatrix, 
        sceneCamera.mProjectionMatrix, vp); // far plane

    resultRay.mOrigin = v0;
    resultRay.mDirection = glm::normalize(v1 - v0);
    return true;
}

void Scene::CollectObjectsForRender(SceneRenderLists& renderList)
{
    CollectObjectsForRender(mCamera, renderList);
}

void Scene::CollectObjectsForRender(Camera& camera, SceneRenderLists& renderList)
{
    BuildObjectsAABBTree(); // force update aabbtree
    camera.ComputeMatricesAndFrustum(gRenderDevice.GetViewport());
    mAABBTree.QueryObjects(camera.mFrustum, [&camera, &renderList, this](SceneObject* object)
    {
        // check object visibility for camera
        if (!camera.mRenderLayers.HasAny(object->GetRenderLayers()))
            return;

        // submit for render
        float distanceToCamera2 = glm::length2(object->GetPosition() - camera.mPosition);
        object->RegisterForRendering(renderList, distanceToCamera2);
    });
}

void Scene::BuildObjectsAABBTree()
{
    while (!mChangedObjects.empty())
    {
        SceneObject* object = mChangedObjects.back();
        mChangedObjects.pop_back();

        // refresh aabbtree node
        mAABBTree.UpdateObject(object);
    }
}

bool Scene::QueryObjects(const cxx::ray3d_t& ray, cxx::any_vector<SceneObject*> queryResult)
{
    return QueryObjects(ray, mCamera, queryResult);
}

bool Scene::QueryObjects(const cxx::ray3d_t& ray, Camera& camera, cxx::any_vector<SceneObject*> queryResult)
{
    queryResult.reserve(32);

    BuildObjectsAABBTree(); // force update aabbtree
    camera.ComputeMatricesAndFrustum(gRenderDevice.GetViewport());
    mAABBTree.QueryObjects(ray, [&camera, this, &queryResult](SceneObject* object)
    {
        // check object visibility for camera
        if (!camera.mRenderLayers.HasAny(object->GetRenderLayers()))
            return;

        queryResult.push_back(object);
    });
    bool isSuccess = !queryResult.empty();
    return isSuccess;
}

cxx::uniqueptr<EnvironmentMeshObject> Scene::CreateLavaMesh(cxx::span<MapTile*> mapTiles)
{
    EnvironmentMeshObject* objectptr = gEnvironmentMeshObjectsPool.Acquire();
    cxx::uniqueptr<EnvironmentMeshObject> resultObject (objectptr, [](EnvironmentMeshObject* object)
        {
            if (object)
            {
                gEnvironmentMeshObjectsPool.Return(object);
            }
        });
    objectptr->SetScene(this);
    objectptr->ConfigureLava();
    objectptr->ConfigureMapTiles(mapTiles);
    return std::move(resultObject);
}

cxx::uniqueptr<EnvironmentMeshObject> Scene::CreateWaterMesh(cxx::span<MapTile*> mapTiles)
{
    EnvironmentMeshObject* objectptr = gEnvironmentMeshObjectsPool.Acquire();
    cxx::uniqueptr<EnvironmentMeshObject> resultObject (objectptr, [](EnvironmentMeshObject* object)
        {
            if (object)
            {
                gEnvironmentMeshObjectsPool.Return(object);
            }
        });
    objectptr->SetScene(this);
    objectptr->ConfigureWater();
    objectptr->ConfigureMapTiles(mapTiles);
    return std::move(resultObject);
}

cxx::uniqueptr<AnimatingMeshObject> Scene::CreateAnimatingMesh()
{
    static SimplePool<AnimatingMeshObject> animatingMeshesPool = (
        [](AnimatingMeshObject* object)
        {
            object->SetObjectActive(false);
            object->OnRecycle();
        });

    AnimatingMeshObject* objectptr = animatingMeshesPool.Acquire();
    objectptr->SetScene(this);
    cxx::uniqueptr<AnimatingMeshObject> resultObject (objectptr, [](AnimatingMeshObject* object)
        {
            if (object)
            {
                animatingMeshesPool.Return(object);
            }
        });
    return std::move(resultObject);
}

cxx::uniqueptr<ProceduralMeshObject> Scene::CreateProceduralMesh()
{
    static SimplePool<ProceduralMeshObject> proceduralMeshesPool = (
        [](ProceduralMeshObject* object)
        {
            object->SetObjectActive(false);
            object->OnRecycle();
        });

    ProceduralMeshObject* objectptr = proceduralMeshesPool.Acquire();
    objectptr->SetScene(this);
    cxx::uniqueptr<ProceduralMeshObject> resultObject (objectptr, [](ProceduralMeshObject* object)
        {
            if (object)
            {
                proceduralMeshesPool.Return(object);
            }
        });
    return std::move(resultObject);
}

void Scene::OnDebugDraw(DebugRenderer& theDebugRenderer)
{
    if (gDebug.mDrawSceneAabbTree)
    {
        mAABBTree.DebugRender(theDebugRenderer);
    }
}

void Scene::OnAnimatorBecomeActive(SceneObject* object)
{
    bool isValid = object && object->IsObjectActive();
    if (!isValid || cxx::contains(mActiveObjects, object))
    {
        cxx_assert(false);
        return;
    }

    mActiveObjects.push_back(object);
    mAABBTree.InsertObject(object);
}

void Scene::OnAnimatorBecomeInactive(SceneObject* object)
{
    bool isValid = object && !object->IsObjectActive();
    if (!isValid || !cxx::erase(mActiveObjects, object))
    {
        cxx_assert(false);
    }

    cxx::erase(mChangedObjects, object);
    mAABBTree.RemoveObject(object);
}

void Scene::OnObjectInvalidateTransform(SceneObject* object)
{
    bool isValid = object && object->IsObjectActive();
    cxx_assert(isValid);

    if (isValid && !cxx::contains(mChangedObjects, object))
    {
        mChangedObjects.push_back(object);
    }
}

void Scene::OnObjectInvalidateBounds(SceneObject* object)
{
    bool isValid = object && object->IsObjectActive();
    cxx_assert(isValid);

    if (isValid && !cxx::contains(mChangedObjects, object))
    {
        mChangedObjects.push_back(object);
    }
}