#pragma once

#include "SceneDefs.h"

// gamemap tiles selection
class MapTilesSelection: public cxx::noncopyable
{
public:
    // readonly
    Rectangle mSelectionRect;

public:
    void Initialize();
    void Deinit();

    // update current selection
    void UpdateSelection(const Rectangle& selectionRect);
    void UpdateSelection(const Point& selectionPoint);
    void ClearSelection();

private:
    void UpdateSelectionMesh();

private:
    GameObject* mSelectionObject = nullptr;
};