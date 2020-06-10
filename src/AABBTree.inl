#pragma once

template<typename TCallback>
inline void AABBTree::QueryObjectsRecursive(const TreeNode* node, const cxx::frustum_t& cameraFrustum, const TCallback& callback, int& maxObjects) const
{
    // see limits
    if (maxObjects < 1 || !cameraFrustum.contains(node->mBoundingBox))
        return;

    // found leaf node
    if (node->IsLeafNode())
    {
        if (node->mEntity)
        {
            callback(node->mEntity);
            maxObjects--;
        }
        return;
    }

    if (node->mLeftNodeIndex != NULL_TREE_NODE)
    {
        const TreeNode* leftNode = &mTreeNodes[node->mLeftNodeIndex];
        QueryObjectsRecursive(leftNode, cameraFrustum, callback, maxObjects);
    }

    if (node->mRightNodeIndex != NULL_TREE_NODE)
    {
        const TreeNode* rightNode = &mTreeNodes[node->mRightNodeIndex];
        QueryObjectsRecursive(rightNode, cameraFrustum, callback, maxObjects);
    }
}

//////////////////////////////////////////////////////////////////////////

template<typename TCallback>
inline void AABBTree::QueryEntities(const cxx::aabbox& aabbox, const TCallback& callback, int maxObjects) const
{
    // todo
}

template<typename TCallback>
inline void AABBTree::QueryEntities(const cxx::bounding_sphere& sphere, const TCallback& callback, int maxObjects) const
{
    // todo
}

template<typename TCallback>
inline void AABBTree::QueryEntities(const cxx::ray3d& ray, const TCallback& callback, int maxObjects) const
{
    // todo
}

template<typename TCallback>
inline void AABBTree::QueryEntities(const cxx::frustum_t& cameraFrustum, const TCallback& callback, int maxObjects) const
{
    if (mRootNodeIndex == NULL_TREE_NODE || maxObjects < 1)
        return;

    // process tree nodes recursive
    const TreeNode* rootNode = &mTreeNodes[mRootNodeIndex];
    QueryObjectsRecursive(rootNode, cameraFrustum, callback, maxObjects);
}