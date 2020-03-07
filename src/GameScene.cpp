#include "pch.h"
#include "GameScene.h"
#include "SceneObject3D.h"

GameScene gGameScene;

bool GameScene::Initialize()
{
    return true;
}

void GameScene::Deinit()
{
    mAABBTree.Cleanup();
}

void GameScene::UpdateFrame()
{

}

void GameScene::HandleSceneObjectTransformChange(SceneObject3D* sceneEntity)
{
    debug_assert(sceneEntity);
    if (!mTransformEntities.contains(&sceneEntity->mListNodeTransformed))
    {
        mTransformEntities.insert(&sceneEntity->mListNodeTransformed); // queue for update
    }
}

void GameScene::AttachSceneEntity(SceneObject3D* sceneEntity)
{
    debug_assert(sceneEntity);
    // already attached to scene
    if (mSceneEntities.contains(&sceneEntity->mListNodeAttached))
    {
        debug_assert(false);
        return;
    }
    else
    {
        mSceneEntities.insert(&sceneEntity->mListNodeAttached);
    }

    mAABBTree.InsertObject(sceneEntity);
}

void GameScene::DetachSceneEntity(SceneObject3D* sceneEntity)
{
    debug_assert(sceneEntity);

    if (mSceneEntities.contains(&sceneEntity->mListNodeAttached))
    {
        mSceneEntities.remove(&sceneEntity->mListNodeAttached);
    }
    else
    {
        return; // already detached
    }

    if (mTransformEntities.contains(&sceneEntity->mListNodeTransformed))
    {
        mTransformEntities.remove(&sceneEntity->mListNodeTransformed);
    }

    mAABBTree.RemoveObject(sceneEntity);
}

void GameScene::BuildAABBTree()
{
    while (mTransformEntities.has_elements())
    {
        cxx::intrusive_node<SceneObject3D>* entityNode = mTransformEntities.get_head_node();
        mTransformEntities.remove(entityNode);

        // refresh aabbtree node
        SceneObject3D* sceneEntity = entityNode->get_element();
        mAABBTree.UpdateObject(sceneEntity);
    }
}