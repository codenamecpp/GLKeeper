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
    // terrain is always opaque?

    renderList.RegisterObject(eRenderPass_Opaque, this);
}