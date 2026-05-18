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
    gGameRenderer.UnregisterDebugVisualizer(this);
}

void Scene::ClearScene()
{
    // make sure to destroy all active objects
    mChangedObjects.clear();

    cxx_assert(mActiveObjects.empty());

    mAABBTree.Cleanup();
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
        if ((camera.mRenderLayersMask & object->GetRenderLayers()) == 0) return;

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