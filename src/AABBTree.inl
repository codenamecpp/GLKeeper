#pragma once

//////////////////////////////////////////////////////////////////////////

template<typename TCallback>
inline void AABBTree::QueryObjectsRecursive(const TreeNode* node, const cxx::frustum_t& cameraFrustum, const TCallback& callback, int& maxObjects) const
{
    // see limits
    if ((maxObjects < 1) || !cameraFrustum.contains(node->mBoundingBox))
        return;

    // found leaf node
    if (node->IsLeafNode())
    {
        if (node->mObject)
        {
            callback(node->mObject);
            maxObjects--;
        }
        return;
    }

    if (node->mLeftNodeIndex != NULL_TREE_NODE)
    {
        QueryObjectsRecursive(&mTreeNodes[node->mLeftNodeIndex], cameraFrustum, callback, maxObjects);
    }

    if (node->mRightNodeIndex != NULL_TREE_NODE)
    {
        QueryObjectsRecursive(&mTreeNodes[node->mRightNodeIndex], cameraFrustum, callback, maxObjects);
    }
}

template<typename TCallback>
void AABBTree::QueryObjectsRecursive(const TreeNode* node, const cxx::ray3d_t& ray, const TCallback& callback, int& maxObjects) const
{
    float outDistanceNear;
    float outDistanceFar;
    if ((maxObjects < 1) || !cxx::intersects(node->mBoundingBox, ray, outDistanceNear, outDistanceFar))
        return;

    // found leaf node
    if (node->IsLeafNode())
    {
        if (node->mObject)
        {
            callback(node->mObject);
            maxObjects--;
        }
        return;
    }

    if (node->mLeftNodeIndex != NULL_TREE_NODE)
    {
        QueryObjectsRecursive(&mTreeNodes[node->mLeftNodeIndex], ray, callback, maxObjects);
    }

    if (node->mRightNodeIndex != NULL_TREE_NODE)
    {
        QueryObjectsRecursive(&mTreeNodes[node->mRightNodeIndex], ray, callback, maxObjects);
    }
}

//////////////////////////////////////////////////////////////////////////

template<typename TCallback>
inline void AABBTree::QueryObjects(const cxx::aabbox& aabbox, const TCallback& callback, int maxObjects) const
{}

template<typename TCallback>
inline void AABBTree::QueryObjects(const cxx::bounding_sphere& sphere, const TCallback& callback, int maxObjects) const
{}

template<typename TCallback>
inline void AABBTree::QueryObjects(const cxx::ray3d_t& ray, const TCallback& callback, int maxObjects) const
{
    if (mRootNodeIndex == NULL_TREE_NODE || maxObjects < 1)
        return;
        
    // process tree nodes recursive
    const TreeNode* rootNode = &mTreeNodes[mRootNodeIndex];
    QueryObjectsRecursive(rootNode, ray, callback, maxObjects);
}

template<typename TCallback>
inline void AABBTree::QueryObjects(const cxx::frustum_t& cameraFrustum, const TCallback& callback, int maxObjects) const
{
    if (mRootNodeIndex == NULL_TREE_NODE || maxObjects < 1)
        return;

    // process tree nodes recursive
    const TreeNode* rootNode = &mTreeNodes[mRootNodeIndex];
    QueryObjectsRecursive(rootNode, cameraFrustum, callback, maxObjects);
}