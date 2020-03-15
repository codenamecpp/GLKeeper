#include "pch.h"
#include "TerrainMesh.h"
#include "SceneRenderList.h"

TerrainMesh::TerrainMesh()
{
}

TerrainMesh::~TerrainMesh()
{
}

void TerrainMesh::RegisterForRendering(SceneRenderList& renderList)
{
    renderList.RegisterObject(eRenderPass_Opaque, this);
}