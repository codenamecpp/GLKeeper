#pragma once

#include "SimpleTriangleMesh.h"
#include "VertexFormat.h"

// generate unit cube triangle mesh
template<typename TVertexFormat>
inline void SetupUnitCubeTriMesh(SimpleTriangleMesh<TVertexFormat>& triMesh, const glm::vec3& cubePosition)
{
    // unit cube      

    //    v6----- v5  
    //   /|      /|   
    //  v1------v0|   
    //  | |     | |   
    //  | v7----|-v4  
    //  |/      |/    
    //  v2------v3    

    // vertex positions
    const int NumVertices = 8;
    static const glm::vec3 vertices[NumVertices] = 
    {
        glm::vec3( 0.5f,  0.5f,  0.5f), // v0
        glm::vec3(-0.5f,  0.5f,  0.5f), // v1
        glm::vec3(-0.5f, -0.5f,  0.5f), // v2
        glm::vec3( 0.5f, -0.5f,  0.5f), // v3

        glm::vec3( 0.5f, -0.5f, -0.5f), // v4
        glm::vec3( 0.5f,  0.5f, -0.5f), // v5
        glm::vec3(-0.5f,  0.5f, -0.5f), // v6
        glm::vec3(-0.5f, -0.5f, -0.5f), // v7
    };

    const int NumTriangles = 12;
    static const glm::ivec3 triangles[NumTriangles] =
    {
        glm::ivec3(0, 1, 2), glm::ivec3(0, 2, 3), // front
        glm::ivec3(6, 5, 4), glm::ivec3(6, 4, 7), // back
        glm::ivec3(1, 6, 7), glm::ivec3(1, 7, 2), // left
        glm::ivec3(5, 0, 3), glm::ivec3(5, 3, 4), // right
        glm::ivec3(5, 6, 1), glm::ivec3(5, 1, 0), // top
        glm::ivec3(7, 4, 3), glm::ivec3(7, 3, 2), // bottom
    };

    triMesh.Clear();
    triMesh.SetTriangles(triangles, NumTriangles);
    triMesh.SetVertexCount(NumVertices);

    // set position
    for (int ipos = 0; ipos < NumVertices; ++ipos)
    {
        triMesh.mVertices[ipos].mPosition = vertices[ipos] + cubePosition;
    }

    triMesh.ComputeBounds();
}