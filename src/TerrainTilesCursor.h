#pragma once

#include "SceneDefs.h"

// gamemap tiles selection
class TerrainTilesCursor: public cxx::noncopyable
{
public:
    // readonly
    Rectangle mSelectionRect;

public:
    void EnterWorld();
    void ClearWorld();
    void UpdateFrame();

    // update current selection
    void UpdateCursor(const Rectangle& selectionRect);
    void UpdateCursor(const Point& selectionPoint);
    void ClearCursor();

private:
    void SetupCursorMesh();
    void SetupCursorMeshMaterial(ProcMeshComponent* component);

private:
    Entity* mMeshObject = nullptr;

    float mCursorEffectTime = 0.0f;
};