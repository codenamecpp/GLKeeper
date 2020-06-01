#include "pch.h"
#include "MapTilesSelection.h"
#include "GameObject.h"
#include "StaticMeshComponent.h"
#include "GameObjectsManager.h"
#include "TexturesManager.h"

void MapTilesSelection::Initialize()
{
    debug_assert(mSelectionObject == nullptr);
    mSelectionObject = gGameObjectsManager.CreateGameObject();
    if (mSelectionObject)
    {
        mSelectionObject->AddComponent<StaticMeshComponent>();
    }
    debug_assert(mSelectionObject);
}

void MapTilesSelection::Deinit()
{
    if (mSelectionObject)
    {
        gGameObjectsManager.DestroyGameObject(mSelectionObject);
        mSelectionObject = nullptr;
    }
    mSelectionRect.w = 0;
    mSelectionRect.h = 0;
}

void MapTilesSelection::UpdateSelection(const Rectangle& selectionRect)
{
    if (selectionRect == mSelectionRect)
        return;

    mSelectionRect = selectionRect;
    UpdateSelectionMesh();
}

void MapTilesSelection::UpdateSelection(const Point& selectionPoint)
{
    Rectangle rcSelection;
        rcSelection.x = selectionPoint.x;
        rcSelection.y = selectionPoint.y;
        rcSelection.w = 1;
        rcSelection.h = 1;
    UpdateSelection(rcSelection);
}

void MapTilesSelection::ClearSelection()
{
    mSelectionRect.w = 0;
    mSelectionRect.h = 0;
    UpdateSelectionMesh();
}

void MapTilesSelection::UpdateSelectionMesh()
{
    if (mSelectionObject == nullptr)
    {
        debug_assert(false);
        return;
    }

    // it is possible to cache mesh component
    StaticMeshComponent* renderable = mSelectionObject->mRenderableComponent->CastComponent<StaticMeshComponent>();
    if (renderable == nullptr)
    {
        debug_assert(false);
        return;
    }

    renderable->ClearMesh();

    if (mSelectionRect.w == 0 || mSelectionRect.h == 0)
    {
        //renderable->UpdateBounds();
        return;
    }

    cxx::aabbox areaBounds;
    GetMapAreaBounds(mSelectionRect, areaBounds);

    // setup selection trimesh
    renderable->mTriMeshParts.resize(1);
    StaticMeshComponent::TriMeshPart& triMesh = renderable->mTriMeshParts[0];

    /*
                  /1--------/3
                 /  |      / |
                /   |     /  |
                5---------7  |
                |   0- - -| -2
                |  /      |  /
                |/        | /
                4---------6/ 
    */

    glm::vec3 edges[8];
    areaBounds.get_edges(edges);

    auto PushUpQuad = [](StaticMeshComponent::TriMeshPart& trimesh, 
        const glm::vec3& point_start, 
        const glm::vec3& point_end)
    {
        float lineWidth = 0.05f;
        float heightModifier = 0.03f; // to prevent z-fighting

        glm::vec3 direction = point_end - point_start;
        glm::vec3 side_cw  = glm::normalize(glm::vec3( direction.z, direction.y + heightModifier, -direction.x)) * lineWidth;
        glm::vec3 side_ccw = glm::normalize(glm::vec3(-direction.z, direction.y + heightModifier,  direction.x)) * lineWidth;

        Color32 verticesColor = Color32_Blue;
        verticesColor.mA = 255;

        // setup vertices
        Vertex3D quad_vertices[4];
        quad_vertices[0].mPosition = point_start + side_cw;  quad_vertices[0].mColor = verticesColor;
        quad_vertices[1].mPosition = point_end   + side_cw;  quad_vertices[1].mColor = verticesColor;
        quad_vertices[2].mPosition = point_start + side_ccw; quad_vertices[2].mColor = verticesColor;
        quad_vertices[3].mPosition = point_end   + side_ccw; quad_vertices[3].mColor = verticesColor;

        // setup triangles
        int indexStart = (int) trimesh.mVertices.size();
        glm::ivec3 quad_triangles[2] =
        {
            glm::ivec3(indexStart + 2, indexStart + 0, indexStart + 1),
            glm::ivec3(indexStart + 1, indexStart + 3, indexStart + 2),
        };

        trimesh.AppendVertices(quad_vertices, 4);
        trimesh.AppendTriangles(quad_triangles, 2);
    };

    PushUpQuad(triMesh, edges[5], edges[7]);
    PushUpQuad(triMesh, edges[5], edges[1]);
    PushUpQuad(triMesh, edges[1], edges[3]);
    PushUpQuad(triMesh, edges[7], edges[3]);
    PushUpQuad(triMesh, edges[0], edges[2]);
    PushUpQuad(triMesh, edges[0], edges[4]);
    PushUpQuad(triMesh, edges[4], edges[6]);
    PushUpQuad(triMesh, edges[2], edges[6]);

    // setup selection trimesh materials
    MeshMaterial material;
    material.mRenderStates.mIsAlphaBlendEnabled = true;
    material.mRenderStates.mIsFaceCullingEnabled = false;
    material.mRenderStates.mBlendingMode = eBlendingMode_AlphaAdditive;
    material.mDiffuseTexture = gTexturesManager.mWhiteTexture;
    material.mColorMode = eMaterialColorMode_Vertex;
    renderable->SetMeshMaterialsCount(1);
    renderable->SetMeshMaterial(0, material);

    renderable->InvalidateMesh();
    renderable->UpdateBounds();
}
