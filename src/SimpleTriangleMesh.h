#pragma once

// simple triangle mesh

#include "VertexFormat.h"

template<typename TVertexFormat>
class SimpleTriangleMesh
{
public:
    using VertexType = typename TVertexFormat::TVertexType;

    // ctor
    SimpleTriangleMesh() = default;
    SimpleTriangleMesh(int vertexCount, int triangleCount)
    {
        mTriangles.resize(triangleCount);
        mVertices.resize(vertexCount);
    }

    // enlarge allocated vertices count to the required size
    // @param vertexCount: Required number of vertices
    inline void EnsureVertexCount(int vertexCount)
    {
        int currentCount = (int) mVertices.size();
        if (currentCount < vertexCount)
        {
            mVertices.resize(vertexCount);
        }
    }

    // enlarge allocated triangles count to the required size
    // @param triangleCount: Required number of triangles
    inline void EnsureTriangleCount(int triangleCount)
    {
        int currentCount = (int) mTriangles.size();
        if (currentCount < triangleCount)
        {
            mTriangles.resize(triangleCount);
        }
    }

    // add vertex data
    // @param vertexData: Source
    // @param vertexCount: Number of vertices in source array
    inline void AppendVertices(const VertexType* vertexData, int vertexCount)
    {
        debug_assert(vertexData);
        mVertices.reserve(vertexCount + mVertices.size());
        for (int ivert = 0; ivert < vertexCount; ++ivert)
        {
            mVertices.push_back(vertexData[ivert]);
        }
    }

    // add triangle data
    // @param vertexData: Source
    // @param vertexCount: Number of vertices in source array
    inline void AppendTriangles(const glm::ivec3* triangleData, int triangleCount)
    {
        debug_assert(triangleData);
        mTriangles.reserve(triangleCount + mTriangles.size());
        for (int itri = 0; itri < triangleCount; ++itri)
        {
            mTriangles.push_back(triangleData[itri]);
        }
    }

    // set vertex data
    // @param vertexData: Source
    // @param vertexCount: Number of vertices in source array
    inline void SetVertices(const VertexType* vertexData, int vertexCount)
    {
        debug_assert(vertexData);
        debug_assert(vertexCount > 0);

        mVertices.assign(vertexData, vertexData + vertexCount);
    }

    // set triangle data
    // @param triangleData: Source
    // @param triangleCount: Number of triangles in source array
    inline void SetTriangles(const glm::ivec3* triangleData, int triangleCount)
    {
        debug_assert(triangleData);
        debug_assert(triangleCount > 0);

        mTriangles.assign(triangleData, triangleData + triangleCount);
    }

    // change number of allocated vertices
    // @param vertexCount: New vertex count
    inline void SetVertexCount(int vertexCount)
    {
        mVertices.resize(vertexCount);
    }

    // change number of allocated triangles
    // @param triangleCount: New triangle count
    inline void SetTriangleCount(int triangleCount)
    {
        mTriangles.resize(triangleCount);
    }

    // refresh bounding box of triangle mesh
    inline void ComputeBounds()
    {
        mBoundingBox.clear();
        for (const VertexType& currVertex: mVertices)
        {
            mBoundingBox.extend(currVertex.mPosition);
        }
    }

    // clear all vertices and triangles
    inline void Clear()
    {
        mTriangles.clear();
        mVertices.clear();
        mBoundingBox.clear();
    }

public:
    using TrianglesArray = std::vector<glm::ivec3>;
    TrianglesArray mTriangles;

    using VerticesArray = std::vector<VertexType>;
    VerticesArray mVertices;

    cxx::aabbox mBoundingBox;
};

// alias
using Vertex3D_TriMesh = SimpleTriangleMesh<Vertex3D_Format>;