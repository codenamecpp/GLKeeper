#pragma once

//////////////////////////////////////////////////////////////////////////

#include "GameDefs.h"
#include "ProceduralMeshObject.h"

//////////////////////////////////////////////////////////////////////////

class MapSelectionCursor: public cxx::noncopyable
{
public:

    //////////////////////////////////////////////////////////////////////////
    enum eSelectionTint 
    { 
        eSelectionTint_Blue, 
        eSelectionTint_Red,
    };
    //////////////////////////////////////////////////////////////////////////

public:
    void Init(Scene& scene);
    void Deinit();

    void UpdateFrame();

    void UpdateSelection(const Rect2D& selectionArea);
    void UpdateSelection(const Point2D& selectionArea);
    void ClearSelection();

    void SetSelectionTint(eSelectionTint tint);
    void ResetSelectionTint();

    inline bool HasSelection() const 
    { 
        return mSelectionArea.HasSize(); 
    }

private:
    void RebuildCursorMesh();
    void UpdateFadingAnimation(float deltaTime);

    static Color32 GetColorForTint(eSelectionTint tint);
        
private:
    cxx::uniqueptr<ProceduralMeshObject> mMeshObject;
    Rect2D mSelectionArea {0, 0, 0, 0};

    eSelectionTint mSelectionTint = eSelectionTint_Blue;
    float mFadingTime = 0.0f;
};

//////////////////////////////////////////////////////////////////////////