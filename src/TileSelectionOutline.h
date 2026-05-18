#pragma once

//////////////////////////////////////////////////////////////////////////

#include "GameDefs.h"
#include "ProceduralMeshObject.h"

//////////////////////////////////////////////////////////////////////////

class TileSelectionOutline: public cxx::noncopyable
{
public:
    enum class SelectionTint { Neutral, Dangerous };

public:
    void Init(Scene& scene);
    void Deinit();

    void UpdateFrame();

    void UpdateSelection(const MapArea2D& selectionArea);
    void UpdateSelection(const MapPoint2D& selectionArea);
    void ClearSelection();

    void SetSelectionTint(SelectionTint tint);

    inline bool HasSelection() const 
    { 
        return (mSelectionArea.w > 0) && (mSelectionArea.h > 0); 
    }

private:
    void RebuildCursorMesh();
    void UpdateFadingAnimation(float deltaTime);

    static Color32 GetColorForTint(SelectionTint tint);
        
private:
    cxx::uniqueptr<ProceduralMeshObject> mMeshObject;
    MapArea2D mSelectionArea {0, 0, 0, 0};

    SelectionTint mSelectionTint = SelectionTint::Neutral;
    float mFadingTime = 0.0f;
};