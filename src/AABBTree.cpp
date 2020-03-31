#include "pch.h"
#include "AABBTree.h"
#include "DebugRenderer.h"
#include "SceneObject.h"

//////////////////////////////////////////////////////////////////////////
// AABBTree implementation borrowed from here
// https://github.com/JamesRandall/SimpleVoxelEngine/blob/master/voxelEngine/src/AABBTree.h
//////////////////////////////////////////////////////////////////////////

AABBTree::AABBTree(unsigned int initialSize)
    : mRootNodeIndex(NULL_TREE_NODE)
    , mAllocatedCount()
    , mNextFreeNodeIndex()
    , mCapacity(initialSize)
    , mGrowthSize(initialSize)
{
    debug_assert(initialSize > 1);
    mTreeNodes.resize(initialSize);
    // setup initial nodes
    for (unsigned int inode = 0; inode < initialSize; ++inode)
    {
        TreeNode& treeNode = mTreeNodes[inode];
        treeNode.mNextNodeIndex = inode + 1;
    }
    mTreeNodes[initialSize - 1].mNextNodeIndex = NULL_TREE_NODE;
}

void AABBTree::AllocateTreeNode(TreeNodeIndex* treeNodeIndex)
{
    debug_assert(treeNodeIndex);

    // no free nodes, allocate additional memory
    if (mNextFreeNodeIndex == NULL_TREE_NODE)
    {
        debug_assert(mAllocatedCount == mCapacity);
        mCapacity += mGrowthSize;

        mTreeNodes.resize(mCapacity);
        // setup nodes
        for (unsigned int inode = mAllocatedCount; inode < mCapacity; ++inode)
        {
            TreeNode& treeNode = mTreeNodes[inode];
            treeNode.mNextNodeIndex = inode + 1;
        }
        mTreeNodes[mCapacity - 1].mNextNodeIndex = NULL_TREE_NODE;
        mNextFreeNodeIndex = mAllocatedCount;
    }

    *treeNodeIndex = mNextFreeNodeIndex;

    TreeNode& treeNode = mTreeNodes[mNextFreeNodeIndex];
    treeNode.mParentNodeIndex = NULL_TREE_NODE;
    treeNode.mLeftNodeIndex = NULL_TREE_NODE;
    treeNode.mRightNodeIndex = NULL_TREE_NODE;
    mNextFreeNodeIndex = treeNode.mNextNodeIndex;
    mAllocatedCount++;
}

void AABBTree::DeallocateTreeNode(TreeNodeIndex treeNodeIndex)
{
    debug_assert(treeNodeIndex != NULL_TREE_NODE);

    TreeNode& treeNode = mTreeNodes[treeNodeIndex];
    treeNode.mNextNodeIndex = mNextFreeNodeIndex;
    mNextFreeNodeIndex = treeNodeIndex;
    mAllocatedCount--;
}

void AABBTree::InsertLeaf(TreeNodeIndex leafNodeIndex)
{
    debug_assert(leafNodeIndex != NULL_TREE_NODE);

	// make sure we're inserting a new leaf
	debug_assert(mTreeNodes[leafNodeIndex].mParentNodeIndex == NULL_TREE_NODE);
	debug_assert(mTreeNodes[leafNodeIndex].mLeftNodeIndex == NULL_TREE_NODE);
	debug_assert(mTreeNodes[leafNodeIndex].mRightNodeIndex == NULL_TREE_NODE);

	// if the tree is empty then we make the root the leaf
	if (mRootNodeIndex == NULL_TREE_NODE)
	{
		mRootNodeIndex = leafNodeIndex;
		return;
	}

	// search for the best place to put the new leaf in the tree
	// we use surface area and depth as search heuristics
	TreeNodeIndex treeNodeIndex = mRootNodeIndex;
	TreeNode& leafNode = mTreeNodes[leafNodeIndex];
	while (!mTreeNodes[treeNodeIndex].IsLeafNode())
	{
		// because of the test in the while loop above we know we are never a leaf inside it
		const TreeNode& treeNode = mTreeNodes[treeNodeIndex];
		TreeNodeIndex leftNodeIndex = treeNode.mLeftNodeIndex;
		TreeNodeIndex rightNodeIndex = treeNode.mRightNodeIndex;
		const TreeNode& leftNode = mTreeNodes[leftNodeIndex];
		const TreeNode& rightNode = mTreeNodes[rightNodeIndex];

        float combinedAabbSurfaceArea = treeNode.mBoundingBox.union_with(leafNode.mBoundingBox).get_surface_area();
		float newParentNodeCost = 2.0f * combinedAabbSurfaceArea;
		float minimumPushDownCost = 2.0f * (combinedAabbSurfaceArea - treeNode.mBoundingBox.get_surface_area());
		// use the costs to figure out whether to create a new parent here or descend
		float costLeft;
		float costRight;
		if (leftNode.IsLeafNode())
		{
			costLeft = leafNode.mBoundingBox.union_with(leftNode.mBoundingBox).get_surface_area() + minimumPushDownCost;
		}
		else
		{
			cxx::aabbox newLeftAabb = leafNode.mBoundingBox.union_with(leftNode.mBoundingBox);
			costLeft = (newLeftAabb.get_surface_area() - leftNode.mBoundingBox.get_surface_area()) + minimumPushDownCost;			
		}
		if (rightNode.IsLeafNode())
		{
			costRight = leafNode.mBoundingBox.union_with(rightNode.mBoundingBox).get_surface_area() + minimumPushDownCost;
		}
		else
		{
			cxx::aabbox newRightAabb = leafNode.mBoundingBox.union_with(rightNode.mBoundingBox);
			costRight = (newRightAabb.get_surface_area() - rightNode.mBoundingBox.get_surface_area()) + minimumPushDownCost;
		}

		// if the cost of creating a new parent node here is less than descending in either direction then
		// we know we need to create a new parent node, errrr, here and attach the leaf to that
		if (newParentNodeCost < costLeft && newParentNodeCost < costRight)	
			break;

		// otherwise descend in the cheapest direction
		if (costLeft < costRight)
		{
			treeNodeIndex = leftNodeIndex;
		}
		else
		{
			treeNodeIndex = rightNodeIndex;
		}
	}

	// the leafs sibling is going to be the node we found above and we are going to create a new
	// parent node and attach the leaf and this item
	TreeNodeIndex leafSiblingIndex = treeNodeIndex;
	TreeNode& leafSibling = mTreeNodes[leafSiblingIndex];
	TreeNodeIndex oldParentIndex = leafSibling.mParentNodeIndex;
	TreeNodeIndex newParentIndex = NULL_TREE_NODE;
    AllocateTreeNode(&newParentIndex);

	TreeNode& newParent = mTreeNodes[newParentIndex];
	newParent.mParentNodeIndex = oldParentIndex;
	newParent.mBoundingBox = leafNode.mBoundingBox.union_with(leafSibling.mBoundingBox); // the new parents aabb is the leaf aabb combined with it's siblings aabb
	newParent.mLeftNodeIndex = leafSiblingIndex;
	newParent.mRightNodeIndex = leafNodeIndex;
	leafNode.mParentNodeIndex = newParentIndex;
	leafSibling.mParentNodeIndex = newParentIndex;

	if (oldParentIndex == NULL_TREE_NODE)
	{
		// the old parent was the root and so this is now the root
		mRootNodeIndex = newParentIndex;
	}
	else
	{
		// the old parent was not the root and so we need to patch the left or right index to
		// point to the new node
		TreeNode& oldParent = mTreeNodes[oldParentIndex];
		if (oldParent.mLeftNodeIndex == leafSiblingIndex)
		{
			oldParent.mLeftNodeIndex = newParentIndex;
		}
		else
		{
			oldParent.mRightNodeIndex = newParentIndex;
		}
	}

	// finally we need to walk back up the tree fixing heights and areas
	treeNodeIndex = leafNode.mParentNodeIndex;
	FixUpwardsTree(treeNodeIndex);
}

void AABBTree::RemoveLeaf(TreeNodeIndex leafNodeIndex)
{
    debug_assert(leafNodeIndex != NULL_TREE_NODE);
	// if the leaf is the root then we can just clear the root pointer and return
	if (leafNodeIndex == mRootNodeIndex)
	{
		mRootNodeIndex = NULL_TREE_NODE;
		return;
	}

	TreeNode& leafNode = mTreeNodes[leafNodeIndex];
	TreeNodeIndex parentNodeIndex = leafNode.mParentNodeIndex;
	const TreeNode& parentNode = mTreeNodes[parentNodeIndex];
	TreeNodeIndex grandParentNodeIndex = parentNode.mParentNodeIndex;
	TreeNodeIndex siblingNodeIndex = parentNode.mLeftNodeIndex == leafNodeIndex ? parentNode.mRightNodeIndex : parentNode.mLeftNodeIndex;
	debug_assert(siblingNodeIndex != NULL_TREE_NODE); // we must have a sibling

	TreeNode& siblingNode = mTreeNodes[siblingNodeIndex];
	if (grandParentNodeIndex != NULL_TREE_NODE)
	{
		// if we have a grand parent (i.e. the parent is not the root) then destroy the parent and connect the sibling to the grandparent in its
		// place
		TreeNode& grandParentNode = mTreeNodes[grandParentNodeIndex];
		if (grandParentNode.mLeftNodeIndex == parentNodeIndex)
		{
			grandParentNode.mLeftNodeIndex = siblingNodeIndex;
		}
		else
		{
			grandParentNode.mRightNodeIndex = siblingNodeIndex;
		}
		siblingNode.mParentNodeIndex = grandParentNodeIndex;
		DeallocateTreeNode(parentNodeIndex);
		FixUpwardsTree(grandParentNodeIndex);
	}
	else
	{
		// if we have no grandparent then the parent is the root and so our sibling becomes the root and has it's parent removed
		mRootNodeIndex = siblingNodeIndex;
		siblingNode.mParentNodeIndex = NULL_TREE_NODE;
		DeallocateTreeNode(parentNodeIndex);
	}

	leafNode.mParentNodeIndex = NULL_TREE_NODE;
}

void AABBTree::UpdateLeaf(TreeNodeIndex leafNodeIndex, const cxx::aabbox& boundingBox)
{
    debug_assert(leafNodeIndex != NULL_TREE_NODE);
	TreeNode& node = mTreeNodes[leafNodeIndex];
	
	// if the node contains the new aabb then we just leave things
	// TODO: when we add velocity this check should kick in as often an update will lie within the velocity fattened initial aabb
	// to support this we might need to differentiate between velocity fattened aabb and actual aabb
	if (node.mBoundingBox.contains(boundingBox)) 
        return;

	RemoveLeaf(leafNodeIndex);
	node.mBoundingBox = boundingBox;
	InsertLeaf(leafNodeIndex);
}

void AABBTree::FixUpwardsTree(TreeNodeIndex treeNodeIndex)
{
    debug_assert(treeNodeIndex != NULL_TREE_NODE);
	while (treeNodeIndex != NULL_TREE_NODE)
	{
		TreeNode& treeNode = mTreeNodes[treeNodeIndex];

		// every node should be a parent
		debug_assert(treeNode.mLeftNodeIndex != NULL_TREE_NODE && treeNode.mRightNodeIndex != NULL_TREE_NODE);

		// fix height and area
		const TreeNode& leftNode = mTreeNodes[treeNode.mLeftNodeIndex];
		const TreeNode& rightNode = mTreeNodes[treeNode.mRightNodeIndex];
		treeNode.mBoundingBox = leftNode.mBoundingBox.union_with(rightNode.mBoundingBox);

		treeNodeIndex = treeNode.mParentNodeIndex;
	}
}

void AABBTree::DebugRender(DebugRenderer& renderer)
{
    if (mRootNodeIndex != NULL_TREE_NODE)
    {
        DebugRenderNode(renderer, mTreeNodes[mRootNodeIndex]);
    }
}

void AABBTree::DebugRenderNode(DebugRenderer& renderer, TreeNode& treeNode)
{
    if (treeNode.IsLeafNode())
    {
        renderer.DrawAabb(treeNode.mBoundingBox, treeNode.mSceneEntity->mDebugColor, true);
        return;
    }
    if (treeNode.mLeftNodeIndex != NULL_TREE_NODE)
    {
        DebugRenderNode(renderer, mTreeNodes[treeNode.mLeftNodeIndex]);
    }

    if (treeNode.mRightNodeIndex != NULL_TREE_NODE)
    {
        DebugRenderNode(renderer, mTreeNodes[treeNode.mRightNodeIndex]);
    }
}

void AABBTree::UpdateTree()
{
    // todo
}

void AABBTree::Cleanup()
{
    // drop all entities links
    mEntitiesMap.clear();

    mRootNodeIndex = NULL_TREE_NODE;
    mNextFreeNodeIndex = 0;
    // setup initial nodes
    for (unsigned int inode = 0; inode < mCapacity; ++inode)
    {
        TreeNode& treeNode = mTreeNodes[inode];
        treeNode.mNextNodeIndex = inode + 1;
    }
    mTreeNodes[mCapacity - 1].mNextNodeIndex = NULL_TREE_NODE;
}

void AABBTree::InsertObject(SceneObject* entity)
{
    debug_assert(entity);
    SceneObjectTransform* transformComponent = entity->GetTransformComponent();
    transformComponent->ComputeTransformation();

    TreeNodeIndex nodeIndex = NULL_TREE_NODE;
    AllocateTreeNode(&nodeIndex);
    
    TreeNode& treeNode = mTreeNodes[nodeIndex];
    treeNode.mBoundingBox = transformComponent->mBoundsTransformed;
    treeNode.mSceneEntity = entity;

    InsertLeaf(nodeIndex);
    mEntitiesMap[entity] = nodeIndex;
}

void AABBTree::RemoveObject(SceneObject* entity)
{
    debug_assert(entity);
    TreeNodeIndex nodeIndex = mEntitiesMap[entity];
    mEntitiesMap.erase(entity);

    RemoveLeaf(nodeIndex);
    DeallocateTreeNode(nodeIndex);
}

void AABBTree::UpdateObject(SceneObject* entity)
{
    debug_assert(entity);
    SceneObjectTransform* transformComponent = entity->GetTransformComponent();
    transformComponent->ComputeTransformation();

    TreeNodeIndex nodeIndex = mEntitiesMap[entity];
    UpdateLeaf(nodeIndex, transformComponent->mBoundsTransformed);
}