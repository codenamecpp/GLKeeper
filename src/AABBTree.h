#pragma once

//////////////////////////////////////////////////////////////////////////
// AABBTree implementation
// https://github.com/JamesRandall/SimpleVoxelEngine/blob/master/voxelEngine/src/AABBTree.h
//////////////////////////////////////////////////////////////////////////

class DebugRenderer;
class Entity;

//////////////////////////////////////////////////////////////////////////
// Bounding Volume Hierarchy Tree
//////////////////////////////////////////////////////////////////////////

class AABBTree
{
public:
    static const int MaxQueryEntities = 16384;

public:
    AABBTree(unsigned int initialSize = 1024);

    // Draw debug information
    // @param renderer: Debug renderer
    void DebugRender(DebugRenderer& renderer);

    // Add spatial object to tree
    // @param entity: Object
    void InsertEntity(Entity* entity);

    // Remove spatial object from tree
    // @param entity: Object
    void RemoveEntity(Entity* entity);

    // Update spatial object location in tree
    // @param entity: Object
    void UpdateEntity(Entity* entity);

    // Get all objects within bounding volume
    // @param aabbox: Bounding box
    // @param maxObjects: Maximum objects
    template<typename TCallback>
    void QueryEntities(const cxx::aabbox& aabbox, const TCallback& callback, int maxObjects = MaxQueryEntities) const;

    // Get all objects within bounding volume
    // @param sphere: Bounding sphere
    // @param maxObjects: Maximum objects
    template<typename TCallback>
    void QueryEntities(const cxx::bounding_sphere& sphere, const TCallback& callback, int maxObjects = MaxQueryEntities) const;

    // Get all objects colliding with ray
    // @param ray: Ray
    // @param maxObjects: Maximum objects
    template<typename TCallback>
    void QueryEntities(const cxx::ray3d& ray, const TCallback& callback, int maxObjects = MaxQueryEntities) const;

    // Get all objects that currently visible
    // @param cameraFrustum: Camera frustum
    // @param maxObjects: Maximum objects
    template<typename TCallback>
    void QueryEntities(const cxx::frustum_t& cameraFrustum, const TCallback& callback, int maxObjects = MaxQueryEntities) const;

    // Rebuild whole tree
    void UpdateTree();

    // Destroy all internal nodes and reset state
    void Cleanup();

private:

    using TreeNodeIndex = unsigned int;
    struct TreeNode;

    // internals
    void AllocateTreeNode(TreeNodeIndex* treeNodeIndex);
    void DeallocateTreeNode(TreeNodeIndex treeNodeIndex);
    void InsertLeaf(TreeNodeIndex treeNodeIndex);
    void RemoveLeaf(TreeNodeIndex treeNodeIndex);
    void UpdateLeaf(TreeNodeIndex treeNodeIndex, const cxx::aabbox& boundingBox);
    void FixUpwardsTree(TreeNodeIndex treeNodeIndex);

    template<typename TCallback>
    void QueryObjectsRecursive(const TreeNode* exploreNode, const cxx::frustum_t& cameraFrustum, const TCallback& callback, int& maxObjects) const;

    void DebugRenderNode(DebugRenderer& renderer, TreeNode& treeNode);

    //////////////////////////////////////////////////////////////////////////

    static const TreeNodeIndex NULL_TREE_NODE = 0xFFFFFFFFUL;

    //////////////////////////////////////////////////////////////////////////

    // Tree internal node data
    struct TreeNode
    {
    public:
        TreeNode()
            : mEntity()
            , mParentNodeIndex(NULL_TREE_NODE)
            , mLeftNodeIndex(NULL_TREE_NODE)
            , mRightNodeIndex(NULL_TREE_NODE)
            , mNextNodeIndex(NULL_TREE_NODE)
        {}

        // test is tree node is leaf
        inline bool IsLeafNode() const { return mLeftNodeIndex == NULL_TREE_NODE; }

    public:
        cxx::aabbox mBoundingBox; // world space aabb
        Entity* mEntity;
        TreeNodeIndex mParentNodeIndex;
        TreeNodeIndex mLeftNodeIndex;
        TreeNodeIndex mRightNodeIndex;
        TreeNodeIndex mNextNodeIndex;
    };
    //////////////////////////////////////////////////////////////////////////

    std::unordered_map<Entity*, TreeNodeIndex> mEntitiesMap;
    std::vector<TreeNode> mTreeNodes;
    TreeNodeIndex mRootNodeIndex;
    TreeNodeIndex mNextFreeNodeIndex;
    unsigned int mAllocatedCount;
    unsigned int mCapacity;
    unsigned int mGrowthSize;
};

#include "AABBTree.inl"