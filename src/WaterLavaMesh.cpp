#include "pch.h"
#include "WaterLavaMesh.h"
#include "SceneRenderList.h"

WaterLavaMesh::WaterLavaMesh()
{
}

WaterLavaMesh::~WaterLavaMesh()
{
}

void WaterLavaMesh::UpdateFrame(float deltaTime)
{
    // todo
}

void WaterLavaMesh::RegisterForRendering(SceneRenderList& renderList)
{
    if (mTranslucency < 1.0f)
    {
        renderList.RegisterObject(eRenderPass_Translucent, this);
    }
    else
    {
        renderList.RegisterObject(eRenderPass_Opaque, this);
    }
}