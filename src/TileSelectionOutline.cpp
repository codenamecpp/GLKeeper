#include "stdafx.h"
#include "TileSelectionOutline.h"
#include "GameWorld.h"
#include "MapUtils.h"
#include "ProceduralMeshObject.h"
#include "TextureManager.h"
#include "Scene.h"

void TileSelectionOutline::Init(Scene& scene)
{
    mMeshObject = scene.CreateProceduralMesh();
    cxx_assert(mMeshObject);

    mSelectionTint = SelectionTint::Neutral; // set to default
    mFadingTime = 0.0f;

    if (mMeshObject)
    {
        mMeshObject->mDebugColor.mA = 0; // hide debug box

        // init material
        SurfaceMaterial& material = mMeshObject->GetMaterial();
        {
            material.mRenderStates.mIsDepthWriteEnabled = false;
            material.mRenderStates.mIsFaceCullingEnabled = false;
            material.mRenderStates.EnableAlphaBlend(RENDER_STATES_BLENDMODE_ALPHA_ADDITIVE);
            material.mDiffuseTexture = gTextureManager.GetTexture("Cursor");
            material.mBaseColor = GetColorForTint(mSelectionTint);
        }
    }

    UpdateFadingAnimation(0.0f);
}

void TileSelectionOutline::Deinit()
{
    mSelectionArea.h = 0;
    mSelectionArea.w = 0;
    mMeshObject.reset();
}

void TileSelectionOutline::UpdateFrame()
{
    if (HasSelection())
    {
        float frameDelta = gTime.GetFrameDelta(eGameClock::Realtime);
        UpdateFadingAnimation(frameDelta);
    }
}

void TileSelectionOutline::UpdateSelection(const MapPoint2D& selectionArea)
{
    MapArea2D rcSelection;
    rcSelection.x = selectionArea.x;
    rcSelection.y = selectionArea.y;
    rcSelection.w = 1;
    rcSelection.h = 1;
    UpdateSelection(rcSelection);
}

void TileSelectionOutline::UpdateSelection(const MapArea2D& selectionArea)
{
    if (mSelectionArea == selectionArea) return;
    if ((selectionArea.h == 0) || (selectionArea.w == 0))
    {
        ClearSelection();
        return;
    }
    mSelectionArea = selectionArea;
    RebuildCursorMesh();
}

void TileSelectionOutline::ClearSelection()
{
    if (!HasSelection()) return;

    mSelectionArea.h = 0;
    mSelectionArea.w = 0;
    mFadingTime = 0.0f;

    RebuildCursorMesh();
    UpdateFadingAnimation(0.0f);
}

void TileSelectionOutline::SetSelectionTint(SelectionTint tint)
{
    if (mSelectionTint == tint) return;

    mSelectionTint = tint;
    if (mMeshObject)
    {
        SurfaceMaterial& material = mMeshObject->GetMaterial();
        material.mBaseColor = GetColorForTint(mSelectionTint);
    }
}

void TileSelectionOutline::RebuildCursorMesh()
{
    if (mMeshObject == nullptr) return;

    bool hasSelection = HasSelection();

    mMeshObject->Clear();
    mMeshObject->SetObjectActive(hasSelection);

    if (!hasSelection)
        return;

    cxx::aabbox areaBounds = MapUtils::ComputeBlocksAreaBounds(mSelectionArea);

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

    float displacement = 0.03f;
    // offset
    glm::vec3 center = areaBounds.get_center();
    for (glm::vec3& curr_edge: edges)
    {
        curr_edge.x += (curr_edge.x > center.x) ? displacement : -displacement;
        curr_edge.y += displacement;
        curr_edge.z += (curr_edge.z > center.z) ? displacement : -displacement;
    }

    // mesh data
    std::vector<glm::ivec3>& meshTriangles = mMeshObject->GetTriangles();
    std::vector<Vertex3D>& meshVertices = mMeshObject->GetVertices();

    auto PushSelectionLine = [&center, &meshTriangles, &meshVertices]( 
        const glm::vec3& point_start, 
        const glm::vec3& point_end, bool isDiagonal)
    {
        float lineWidth = 0.03f;
        glm::vec3 direction = isDiagonal ? (point_start - center) : (point_end - point_start);

        glm::vec3 sides[2] =
        {
            isDiagonal ? glm::vec3(direction.y, 0.0f, -direction.x) : glm::vec3( direction.z, 0.0f, -direction.x),
            isDiagonal ? glm::vec3(direction.x, 0.0f,  direction.y) : glm::vec3( 0.0f, direction.z,  direction.x)
        };
        meshVertices.reserve(meshVertices.size() + 2 * 4);

        const Color32 verticesColor = COLOR_WHITE;
        for (const glm::vec3& currSide: sides)
        {
            glm::vec3 side_cw  = glm::normalize(currSide) * lineWidth;
            glm::vec3 side_ccw = glm::normalize(currSide * -1.0f) * lineWidth;
            int indexStart = static_cast<int>(meshVertices.size());
            // setup vertices
            {
                Vertex3D& quad_vertex = meshVertices.emplace_back();
                quad_vertex.mPosition = point_start + side_cw; quad_vertex.mColor = verticesColor; quad_vertex.mTexcoord = {0.0f, 1.0f};
            }
            {
                Vertex3D& quad_vertex = meshVertices.emplace_back();
                quad_vertex.mPosition = point_end + side_cw; quad_vertex.mColor = verticesColor; quad_vertex.mTexcoord = {1.0f, 1.0f};
            }
            {
                Vertex3D& quad_vertex = meshVertices.emplace_back();
                quad_vertex.mPosition = point_start + side_ccw; quad_vertex.mColor = verticesColor; quad_vertex.mTexcoord = {0.0f, 0.0f};
            }
            {
                Vertex3D& quad_vertex = meshVertices.emplace_back();
                quad_vertex.mPosition = point_end + side_ccw; quad_vertex.mColor = verticesColor; quad_vertex.mTexcoord = {1.0f, 0.0f};
            }

            // setup triangles
            meshTriangles.emplace_back(indexStart + 2, indexStart + 0, indexStart + 1);
            meshTriangles.emplace_back(indexStart + 1, indexStart + 3, indexStart + 2);
        }
    };

    PushSelectionLine(edges[5], edges[7], false);
    PushSelectionLine(edges[5], edges[1], false);
    PushSelectionLine(edges[1], edges[3], false);
    PushSelectionLine(edges[7], edges[3], false);
    PushSelectionLine(edges[0], edges[2], false);
    PushSelectionLine(edges[0], edges[4], false);
    PushSelectionLine(edges[4], edges[6], false);
    PushSelectionLine(edges[2], edges[6], false);

    // diagonals
    PushSelectionLine(edges[1], edges[0], true);
    PushSelectionLine(edges[5], edges[4], true);
    PushSelectionLine(edges[7], edges[6], true);
    PushSelectionLine(edges[3], edges[2], true);

    mMeshObject->InvalidateMesh();
}

void TileSelectionOutline::UpdateFadingAnimation(float deltaTime)
{
    mFadingTime += deltaTime;

    if (mMeshObject)
    {
        SurfaceMaterial& material = mMeshObject->GetMaterial();
        material.mOpacity = 0.1f + glm::smoothstep(0.0f, 0.45f, cxx::ping_pong(mFadingTime, 0.3f));
    }
}

Color32 TileSelectionOutline::GetColorForTint(SelectionTint tint)
{
    return (tint == SelectionTint::Dangerous) ? COLOR_RED : COLOR_BLUE;
}

